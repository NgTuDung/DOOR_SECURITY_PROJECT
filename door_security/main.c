#include <i2c.h>
#include <avr/io.h>
#include <stdlib.h> 
#include <string.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include <lcd.h>
#include <PWM.h>
#include <UART.h>
#include <rfid.h>

#define COL_SET PORTA  // Ch?n PORT C làm port c?a c?t
#define ROW_SET PINA   // Ch?n PIN B làm port c?a hàng

#define MAX_LENGTH 4 //Do dai cua mat khau
char default_password[MAX_LENGTH + 1] = "1234"; //Mat khau mac dinh cua he thong

char EEMEM right_password[MAX_LENGTH +1];
uint8_t EEMEM password_flag;

char checked_password[MAX_LENGTH + 1];

// ??nh ngh?a chân c?t và hàng t??ng ?ng
uint8_t Col_pins[4] = {PA3, PA2, PA1, PA0}; // Các chân c?a c?t là PA0-PA3
uint8_t Row_pins[4] = {PA4, PA5, PA6, PA7}; // Các chân c?a hàng là PB0-PB3

// Ma tr?n bàn phím
uint8_t key_maps[4][4] = {
	{1, 2, 3, 10},
	{4, 5, 6, 11},
	{7, 8, 9, 12},
	{14, 0, 15, 13}
};

volatile bool INT0_flag = 0;
volatile bool door_state = 0;


//khai bao thiet bi RC522
MFRC522_Dev rfid;

ISR(INT0_vect) {
	INT0_flag = 1;
	if(door_state == 1) {
		door_state = 0;
	} else {
		door_state = 1;
	}
}


// Ham thuc hien kiem tra mat khau
bool check_password(const char *password, const char *correct_password)
{
	return strcmp(password, correct_password) == 0;
}

void init_keypad()
{
	// Cau hinh cac chan hang PORT A tu A4 - A7 va bat pull_down resistor
	DDRA &= ~((1 << Row_pins[0]) | (1 << Row_pins[1]) | (1 << Row_pins[2]) | (1 << Row_pins[3])); // INPUT
	PORTA &= (1 << Row_pins[0]) | (1 << Row_pins[1]) | (1 << Row_pins[2]) | (1 << Row_pins[3]);  // pull_down resistor
	
	// Cau hinh cac chan cot PORTA tu A0 - A3 la OUTPUT va dat muc LOW
	DDRA |= (1 << Col_pins[0]) | (1 << Col_pins[1]) | (1 << Col_pins[2]) | (1 << Col_pins[3]); // OUTPUT
	PORTA &= ~((1 << Col_pins[0]) | (1 << Col_pins[1]) | (1 << Col_pins[2]) | (1 << Col_pins[3])); // LOW
}

void INT0_init() {
	// Cau hinh ngat INT0 ngat suon xuong
	MCUCR |= (1 << ISC01);  // ISC01 = 1, ISC00 = 0 ? Falling edge
	MCUCR &= ~(1 << ISC00);

	// Enable INT0
	GICR |= (1 << INT0);

	// Xoa co ngat INT0 neu co
	GIFR |= (1 << INTF0);

	// cau hinh INT0 la input, tro keo len
	DDRD &= ~(1 << PD2);    
	PORTD |= (1 << PD2);    
}

void Pass_word_init() {
	 uint8_t flag = eeprom_read_byte(&password_flag);
	if(flag != 0xAA) {
		eeprom_write_block((const void *)default_password, (void *)right_password, MAX_LENGTH);
		eeprom_write_byte(&password_flag, 0xAA);
	}
}

uint8_t press_check(void) {
	uint8_t a=0; 

	for (int i = 0; i < 4; i++) {
		// Thuc hien vong quet dua lan luot cac chan COLUM len muc cao
		COL_SET |= (1 << Col_pins[i]);
		_delay_ms(1);

		for (int j = 0; j < 4; j++) {
			// Thuc hien vong quet lan luot kiem tra tin hieu cac chan ROW
			if (( ROW_SET & (1 << Row_pins[j]) )>0) { 
				a = key_maps[j][i]; // Lay gia tri cua chan COLUM va ROW tuong ung duoc nhan
				_delay_ms(250);             // Delay tranh doi phim
				COL_SET &= ~(1 << Col_pins[i]); // Keo tin hieu cua chan COLUM xuong muc thap
				return a;           // Tra ve gia tri cua nut nhan tuong ung
			}
		}
		// Keo tin hieu cua chan COLUM xuong muc thap
		COL_SET &= ~(1 << Col_pins[i]);
	}

	return 100; //Tra ve mot gia tri rac khi khong co nut nao duoc nhan
}

uint8_t set_new_password(char *correct_password)
{
  char current_password[MAX_LENGTH + 1]; // Mang luu tru mat khau hien tai
  char new_password[MAX_LENGTH + 1];     // Mang luu tru mat khau moi
  
  // Thuc hien nhay coi bao vao chuong trinh nhap mat khau moi
  PORTD |= (1<<PD6);
  _delay_ms(250);
  PORTD &= ~(1<<PD6);
  _delay_ms(125);
  for(uint8_t i=0;i<2;i++){
	  PORTD |= (1<<PD6);
	  _delay_ms(125);
	  PORTD &= ~(1<<PD6);
	  _delay_ms(125);
  }
  
  LCD_clear();
  LCD_print("Enter old pass:");
  LCD_setCursor(0, 1);
  LCD_print("     ");
  
  int current_length = 0; //Bien luu tru do dai mat khau hien tai
  while (current_length < MAX_LENGTH)
    {
      uint8_t key = press_check(); // Bien luu tru gia tri tra ve ham nhan nut
	  
	  if(INT0_flag) { //Kiem tra xem nut nhan mo cua co duoc nhan khong?
		  return 2;
	  }
	  if(UART_flag) { //Kiem tra xem co task duoc gui tu esp khong?
		  return 3;
	  }
	  
      if (key != 100) {
		  
		  //Neu co phim duoc nhan
		  PORTD |= (1<<PD6);
		  _delay_ms(125);
		  PORTD &= ~(1<<PD6);
		  
          if (key == 10)
            { // Phim xoa so truoc do duoc nhap
              if (current_length > 0)
                {
                  current_length--;
                  current_password[current_length] = 0; // THuc hien xoa ky tu cuoi cung
                  
                  LCD_setCursor(current_length, 1);
                  LCD_print(" "); // Xoa ky tu cuoi cung tren LCD
                                  
                }
            }
          else if (key == 13)
            { // Phim ENTER duoc nhan

              if (current_length != MAX_LENGTH)
                {
                      
                      LCD_clear();
                      LCD_print("Password length");
                      LCD_setCursor(0, 1);
                      LCD_print("must be 4 digits!");
                      
					  PORTD |= (1<<PD6);
					  _delay_ms(1500);
					  PORTD &= ~(1<<PD6);
					                     
                      return 0;
                }
              else
                {
                  break;
                } // Ket thuc nhap mat khau hien tai
            }
          else if(key>=0 && key<=9)
            { // Neu la cac phim so duoc nhan
              if(current_length < MAX_LENGTH)
                {
                  current_password[current_length] = key + '0'; // Truyen tu so thanh ky tu
                  
                  LCD_setCursor(current_length, 1);
                  LCD_print("*"); // THem dau * hien thi tren LCD

                  current_length++;
                  current_password[current_length] = '\0';
                }
            }

      }
    }

  // Kiem tra mat khau vua nhap co dung voi mat khau hien tai
  if (!check_password(current_password, correct_password)) {
      
	  // Thuc hien truong trinh nhay coi neu kiem tra sai
      LCD_clear();
      LCD_print("Incorrect current");
      LCD_setCursor(0, 1);
      LCD_print("password!");
	  
	  PORTD |= (1<<PD6);
	  _delay_ms(1000);
	  PORTD &= ~(1<<PD6);
	  _delay_ms(500);

      return 0; //Tra ve thoat khoi chuong trinh nhap mat khau
  }
  
  // Neu nhap dung thuc hien chuong trinh nhay coi vao nhap mat khau moi
  PORTD |= (1<<PD6);
  _delay_ms(250);
  PORTD &= ~(1<<PD6);
  _delay_ms(125);
  for(uint8_t i=0;i<2;i++){
	  PORTD |= (1<<PD6);
	  _delay_ms(125);
	  PORTD &= ~(1<<PD6);
	  _delay_ms(125);
  }

  /**************** Bat dau thuc hien nhap set mat khau moi *****************/

  
  LCD_clear();
  LCD_print("Enter new pass:");
  LCD_setCursor(0, 1);
  LCD_print("     ");
  
  int new_length = 0; // Bien luu tru do dai nhap mat khau moi
  while (1)
    {
      uint8_t key = press_check(); // Bien nhan gia tri tra ve cua ham nhan

	  if(INT0_flag) { //Kiem tra xem nut nhan mo cua co duoc bam khong?
		  return 2;
	  }
	  if(UART_flag) { //Kiem tra xem co task tu esp gui xuong khong?
		  return 3;
	  }

      if (key != 100)
        { //Neu co phim duoc nhan
		  PORTD |= (1<<PD6);
		  _delay_ms(125);
		  PORTD &= ~(1<<PD6);
		  
          if (key == 10)
            { // Phim xoa di so vua nhap truoc do
              if (new_length > 0)
                {
                  new_length--;
                  new_password[new_length] = 0; // Xoa ky tu cuoi cung
                  
                  LCD_setCursor(new_length, 1);
                  LCD_print(" "); // Xoa ky tu cuoi cung tren man hinh LCD
                  //LCD_setCursor(new_length, 1);
                  
                }
            }
          else if (key == 13)
            { // Phim ENTER duoc nhan
              if (new_length == MAX_LENGTH)
                {
					PORTD |= (1<<PD6);
					_delay_ms(250);
					PORTD &= ~(1<<PD6);
					_delay_ms(125);
					for(uint8_t i=0;i<2;i++){
						PORTD |= (1<<PD6);
						_delay_ms(125);
						PORTD &= ~(1<<PD6);
						_delay_ms(125);
					}
					
					break; // Ket thuc nhap mat khau moi
                }
              else
                  {
                      
                      LCD_clear();
                      LCD_print("Password must be");
                      LCD_setCursor(0, 1);
                      LCD_print("4 digits!");
					  
					  PORTD |= (1<<PD6);
					  _delay_ms(1000);
					  PORTD &= ~(1<<PD6);
					  _delay_ms(500);

                      return 0;
                  }
            }
          else if (key >= 0 && key <= 9)
            { // Neu la so duoc nhan
              if (new_length < MAX_LENGTH)
                {
                  new_password[new_length] = key + '0'; // Chuyen doi so thanh ky tu
                  
                  LCD_setCursor(new_length, 1);
                  LCD_print("*"); // Thêm d?u * trên LCD

                  new_length++;
                }
            }
        }
    }

  // Luu mat khau moi vao bien toan cuc - bien luu tru gia tri cua mat khau hien tai
  strncpy(correct_password, new_password, MAX_LENGTH);
  correct_password[MAX_LENGTH] = '\0'; // Dam bao gia tri cuoi cung la gia tri NULL
  
  
  //Thuc hien update mat khau dung trong eerom
  eeprom_update_block((const void *)correct_password, (void *)right_password, MAX_LENGTH);
  
  LCD_clear();
  LCD_print("Password set");
  LCD_setCursor(0, 1);
  LCD_print("successfully!");
  _delay_ms(1500);
  return 1;
}

uint8_t Password_check(char *CoPass)
{
  char password[MAX_LENGTH + 1] = {0}; // Mang chua mat khau nhap vao
  uint8_t length = 0; // Bien do dai cua mat khau

  LCD_clear();
  LCD_print("Enter password:");

  int k= 0; // Bien kiem tra so lan nhap sai mat khau
  while (1) {
      uint8_t key = press_check(); // Doc gia tri tu keypad
	  
	  if(INT0_flag) { //Kiem tra xem nut nhan mo cua co duoc bam khong?
		  return 2;
	  }
	  if(UART_flag) { //Kiem tra xem co task tu esp gui xuong khong?
		  return 3;
	  }
	  
      if (key != 100)
        { 
		  
		  //Neu co phim duoc nhan
		  PORTD |= (1<<PD6);
		  _delay_ms(125);
		  PORTD &= ~(1<<PD6);
		  
          if (key == 10)
            { // Phim xoa so truoc do 
              if (length > 0) {
                  length--;
                  password[length] = 0; // Xoa di ky tu cuoi cung
                  
                  LCD_setCursor(length, 1);
                  LCD_print(" "); // Xoa ky tu cuoi cung cua LCD
                  
              }
            }
          else if (key == 13)
            { // Phim ENTER duoc nhan
              if (length == MAX_LENGTH)
                {
                  if (check_password(password,CoPass))
                    {// Kiem tra neu nhu mat khau nhap dung

                      LCD_clear();
                      LCD_print("Password Right!");
					  
					  // Thuc hien nhay coi khi nhap dung mat khau
					  for(uint8_t i=0;i<5;i++){
						  PORTD |= (1<<PD6);
						  _delay_ms(125);
						  PORTD &= ~(1<<PD6);
						  _delay_ms(125);
					  }
					  
                      return 1; //Tra ve gia tri 1 khi nhap dung mat khau
                    }
                  else
                    {// Neu mat khau nhap sai 

                      LCD_clear();
                      LCD_print("Wrong pass!");
					  
					  // Thuc hien nhay coi khi nhap sai mat khau
					  PORTD |= (1<<PD6);
					  _delay_ms(1000);
					  PORTD &= ~(1<<PD6);
					  _delay_ms(500);

                      k=k+1; //Cong them vao bien kiem tra so lan nhap sai
                    }
                  length = 0; // Dat lai do dai va chuoi
                  memset(password, 0, sizeof(password)); // Xoa mang mat khau


                }
              else
                {// Neu chua nhap du so ky tu la 4 ky tu cua mat khau
				  
                  LCD_clear();
                  LCD_print("Must be 4 digits");
				  
				  // Thuc hien nhay coi khi nhap sai
				  PORTD |= (1<<PD6);
				  _delay_ms(1000);
				  PORTD &= ~(1<<PD6);
				  _delay_ms(500);
				  
                  k=k+1;// Cong them vao bien kiem tra so lan nhap sai
                }
              if(k==5)
                { //Neu thuc hien viec nhap mat khau khong dung qua 5 lan


                  LCD_clear();
                  LCD_print("Wrong 5 times!");
				  
				  PORTD |= (1<<PD6);
				  _delay_ms(3500);
				  PORTD &= ~(1<<PD6);

                  return 0; //Tra ve gia tri khong khi da nhap sai qua 5 lan 
                }
              length = 0; // Dat lai do dai cua chuoi nhap
              memset(password, 0, sizeof(password)); // Xoa mang mat khau

              LCD_clear();
              LCD_print("Enter password:");
            }
          else if(key == 14)
            { // Phim vao phan chuc nang set mat khau moi
			  
			  uint8_t set_check = 0;
              set_check = set_new_password(CoPass);

              length = 0; // Dat lai do dai cua chuoi nhap
              memset(password, 0, sizeof(password)); // Xoa mang mat khau
			  
			  if(set_check == 2) {
				return 2;			  
			  }
              LCD_clear();
              LCD_print("Enter password:");
            }

          else if(key >= 0 && key <= 9)
              { // Neu la so duoc nhan
              if (length < MAX_LENGTH)
                {
                  password[length] = key + '0'; // Chuyen so thanh ky tu
                  
                  LCD_setCursor(length, 1);
                  LCD_print("*"); // Thêm dau * tren LCD

                  length++;
                }
              }
        }
  }
}

uint8_t RFID_check() {
	uint8_t count = 0;
	LCD_clear();
	LCD_print("Scan card:");
	while(1) {
		uint8_t check = RFID_IsValidUID(&rfid);
		
		if(INT0_flag) {
			return 2;
		}
		if(UART_flag) {
			return 3;
		}
		
		if(check == 1) {
			LCD_clear();
			LCD_print("Right Card!");
			for(uint8_t i=0;i<2;i++){
				PORTD |= (1<<PD6);
				_delay_ms(125);
				PORTD &= ~(1<<PD6);
				_delay_ms(125);
			}
			return 1;
		} else if(check == 0) {
			LCD_clear();
			LCD_print("Wrong Card!");
			PORTD |= (1<<PD6);
			_delay_ms(1000);
			PORTD &= ~(1<<PD6);
			count++;
			LCD_clear();
			LCD_print("Scan card:");
		}
		if(count == 5) {
			LCD_clear();
			LCD_print("Wrong 5 times!");
			
			PORTD |= (1<<PD6);
			_delay_ms(3500);
			PORTD &= ~(1<<PD6);
			return 0;
		}
	}
}

void Open_Door() {
	door_state = 1;
	LCD_clear();
	LCD_print("Open! Come In");
	OCR1A = 1900;
	// Thuc hien nhay coi khi nhap dung mat khau
	for(uint8_t i=0;i<2;i++){
		PORTD |= (1<<PD6);
		_delay_ms(125);
		PORTD &= ~(1<<PD6);
		_delay_ms(125);
	}
	UART_sendChar('O'); //Gui thong tin cho esp
	UART_sendString((char *)current_uid); //Gui thong tin cho ESP
	UART_flag = 0;
	uint8_t y = 99;
	while(1){
		y = press_check();
		if(y==0 || y==1 || y==2 || y==3 || y==4 || y==5 || y==6 || y==7 || y==8 || y==9 || y==10 || y==11 || y==12 || y==13 || y==14 || y==15 || 
			door_state == 0)
		{
			break;
		}
		if(UART_flag) {
			_delay_ms(125);
			if(strcmp((char*)uart_buffer, "Close") == 0) {
				break;
			} else {
				UART_flag = 0;
				memset((char *)uart_buffer, 0, UART_BUFFER_SIZE);
			}
		}
		
	}
	UART_flag = 0;
	memset((char *)uart_buffer, 0, UART_BUFFER_SIZE);
	memset((char *)current_uid, 0, UID_LEN*2+1);
}

void Close_Door() {
	door_state = 0;
	INT0_flag = 0;
	LCD_clear();
	LCD_print("Close!");
	OCR1A = 900;
	for(uint8_t i=0;i<2;i++){
		PORTD |= (1<<PD6);
		_delay_ms(125);
		PORTD &= ~(1<<PD6);
		_delay_ms(125);
	}
	UART_sendChar('C'); //Gui thong tin cho ESP
	UART_flag = 0;
	memset((char *)uart_buffer, 0, UART_BUFFER_SIZE);
}

void Access_Denied() {
	door_state = 0;
	INT0_flag = 0;
	LCD_clear();
	LCD_print("Access Denied!");
	UART_sendChar('W');
	while(1){
		if(door_state == 1) { //Co nguoi tu ben trong kich hoat se enable cho cua
			break;
		}
		
		if(UART_flag) {
			
			_delay_ms(125);// Dam bao ngat UART da nhan du chuoi roi moi thuc hien cac phan khac
			
			if(strcmp((char*)uart_buffer, "Access") == 0){
				break;
			} else {
				UART_flag = 0;
				memset((char *)uart_buffer, 0, UART_BUFFER_SIZE);
			}
		}
	}
	memset((char *)uart_buffer, 0, UART_BUFFER_SIZE);
	memset((char *)current_uid, '0', UID_LEN*2+1);
	UART_flag = 0;
}

void Pass_word() {
	door_state = 0;
	INT0_flag = 0;
	LCD_clear();
	LCD_print("Send Password!");
	UART_sendChar('P');
	UART_sendString(checked_password);
	_delay_ms(2000);
	memset((char *)uart_buffer, 0, UART_BUFFER_SIZE);
	memset((char *)current_uid, 0, UID_LEN*2+1);
	UART_flag = 0;
}

void Change_pass_request() {
	door_state = 0;
	INT0_flag = 0;
	LCD_clear();
	LCD_print("Password Changed");
	strncpy(checked_password, (char *)uart_buffer + 2, 4);
	checked_password[MAX_LENGTH + 1] = '\0';
	_delay_ms(2000);
	
	//Thuc hien upadte mat khau moi vao eerom
	eeprom_update_block((const void *)checked_password, (void *)right_password, MAX_LENGTH);
	
	memset((char *)uart_buffer, 0, UART_BUFFER_SIZE);
	memset((char *)current_uid, 0, UID_LEN*2+1);
	UART_flag = 0;
}
int main(void)
{
	const int RST_PIN = 3;
	
	I2C_init(); //Khoi tao cho I2C
	LCD_init();	//Khoi tao cho LCD
	init_keypad(); //Khoi tao cho keypad
	PWM_init(); //Khoi tao cho PWM
	INT0_init(); //Khoi tao ngat ngoai cho INT0
	UART_init(); //Khoi tao truyen nhan UART voi ESP
	sei();
	
	SPI_Init();
	MFRC522_Init(&rfid, SS, RST_PIN);
	MFRC522_Begin(&rfid);

	Pass_word_init();
	
	DDRD |= (1<<PD6); //Khoi tao chan coi buzzer
	PORTD &= ~(1<<PD6);
	
	Close_Door();
	
	while (1)
	{
		// Thuc hien lay mat khau dung trong vung nho EEROM
		eeprom_read_block((void *)checked_password, (const void *)right_password, MAX_LENGTH);
		
		uint8_t check_pass = 99;
		uint8_t check_rfid = 99;
		
		check_rfid = RFID_check();
		if(check_rfid == 1) {
			// Di toi nhap mat khau
		} else if(check_rfid == 0) {
			Access_Denied();
			Close_Door();
			continue;
		} else if(check_rfid == 2) {
			memset((char *)current_uid, 0, UID_LEN*2+1); //TH co nguoi tu trong bam ra thi xoa luon mang uid
			Open_Door();
			Close_Door();
			continue;
		} else if(check_rfid == 3) {
			
			_delay_ms(125);// Dam bao ngat UART da nhan du chuoi roi moi thuc hien cac phan khac
			
			if(strcmp((char*)uart_buffer, "Open") == 0) {
				memset((char *)current_uid, 0, UID_LEN*2+1);//TH esp yeu cau mo thi xoa luon mang uid
				Open_Door();
				Close_Door();
				continue;
			} else if(strcmp((char *)uart_buffer, "Passcheck") == 0) {
				Pass_word();
				continue;
			} else if(uart_buffer[0] == 'P' && uart_buffer[1] == 'C') {
				Change_pass_request();
				continue;
			}
		}
		
		
		check_pass = Password_check(checked_password);
		if(check_pass == 1) {
			Open_Door();
		} else if(check_pass == 2) {
			memset((char *)current_uid, 0, UID_LEN*2+1); // TH co nguoi tu trong bam ra thi xoa luon mang uid
			Open_Door();
		} else if(check_pass == 0) {
			Access_Denied();
		} else if(check_pass == 3) {
			
			_delay_ms(125);// Dam bao ngat UART da nhan du chuoi roi moi thuc hien cac phan khac
			
			if(strcmp((char*)uart_buffer, "Open") == 0) {
				memset((char *)current_uid, 0, UID_LEN*2+1);//TH esp yeu cau mo thi xoa luon mang uid
				Open_Door();
			} else if(strcmp((char *)uart_buffer, "Passcheck") == 0) {
				Pass_word();
			} else if(uart_buffer[0] == 'P' && uart_buffer[1] == 'C') {
				Change_pass_request();
			}
		}
		Close_Door();
		
	}
}

