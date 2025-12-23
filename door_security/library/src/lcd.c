#include "i2c.h" // Include thư viện I2C đã viết
#include "lcd.h"


uint8_t backlight_state = 0x00;

void LCD_sendCommand(uint8_t cmd) {
	uint8_t highNibble = (cmd & 0xF0) | backlight_state; // RS=0
	uint8_t lowNibble = ((cmd << 4) & 0xF0) | backlight_state; //RS=0
	
	// Truyền 4 bits cao
	I2C_start();
	I2C_write(LCD_I2C_ADDRESS << 1); // Vi vdk dang can ghi len Slave cua pcf8574 nen bit cuoi trong bus 8 bit la 0
	I2C_write(highNibble | LCD_EN); 
	I2C_write(highNibble); // Chuyen Enable tu HIGH sang LOW de thuc hien ghi gia tri tu DB4-DB7
	I2C_stop();	
	
	// Truyền 4 bits thấp
	I2C_start();
	I2C_write(LCD_I2C_ADDRESS << 1);
	I2C_write(lowNibble | LCD_EN);
	I2C_write(lowNibble);
	I2C_stop();
}

void LCD_sendData(uint8_t data) {
	int8_t highNibble = (data & 0xF0) | LCD_RS | backlight_state; //RS=1
	uint8_t lowNibble = ((data << 4) & 0xF0) | LCD_RS | backlight_state; //RS=1
	
	// Truyền 4 bits cao
	I2C_start();
	I2C_write(LCD_I2C_ADDRESS << 1);
	I2C_write(highNibble | LCD_EN);
	I2C_write(highNibble);
	I2C_stop();
	
	// Truyền 4 bits thấp
	I2C_start();
	I2C_write(LCD_I2C_ADDRESS << 1);
	I2C_write(lowNibble | LCD_EN);
	I2C_write(lowNibble);
	I2C_stop();
}

void LCD_print(char *str) {
	while (*str) {
		LCD_sendData(*str++);
	}
}

void LCD_clear() {
	LCD_sendCommand(0x01);
	_delay_ms(2);
}

void LCD_DisAndCurControl(bool a, bool b, bool c) {
	if(a==0 && b==0) {
		LCD_sendCommand(0x08); // Display Off, Cursor off
	} else if (a==1 && b==0) {
		LCD_sendCommand(0x0C); // Display On, Cursor Off
	} else if (a==0 && b==1) {
		if(c==0) {
			LCD_sendCommand(0x0A); // Display Off, Cursor On, Cursor Blink Off
		} else if(c==1) {
			LCD_sendCommand(0x0B); // Display Off, Cursor On, Cursor Blink On
		}
	} else if(a==1 && b==1) {
		if (c == 0) {
			LCD_sendCommand(0x0E); // Display On, Cursor On, Cursor Blink Off
		} else if(c==1) {
			LCD_sendCommand(0x0F); // Display On, Cursor On, Cursor Blink On
		}
		
	}
}

void LCD_Shift(bool a, bool b) {
	if (a == DISPLAY && b == LEFT) {
		LCD_sendCommand(0x18); // Dich chuyen toan bo ky tu sang trai
	} else if (a == DISPLAY && b == RIGHT) {
		LCD_sendCommand(0x1C); // Dich chuyen toan bo ky tu sang phai
	} else if (a == CURSOR && b == LEFT) {
		LCD_sendCommand(0x10); // Dich chuyen vi tri con tro sang trai
	} else if(a == CURSOR && b == RIGHT) {
		LCD_sendCommand(0x14); // Dich chuyen vi tri con tro sang phai
	}
}

void LCD_Backlight(bool a) {
	if (a==1) {
		backlight_state = 0x08; // Dao trang thai cho chan p3 len 1
	} else if (a==0) {
		backlight_state = 0x00; // Dao trang thai cho chan p3 ve 0
	}
	I2C_start();
	I2C_write(LCD_I2C_ADDRESS << 1);
	I2C_write(backlight_state);
	I2C_stop();
}

void LCD_setCursor(uint8_t col, uint8_t row) {
	uint8_t address;
	// Tính toán địa chỉ cho dòng 0 và dòng 1
	if (row == 0) {
		address = 0x80 + col;  // Dòng 0
		} else {
		address = 0xC0 + col;  // Dòng 1
	}
	LCD_sendCommand(address);  // Gửi lệnh đặt con trỏ
}

void LCD_init() {
		
	_delay_ms(20);  
	LCD_sendCommand (0x02);	
	LCD_sendCommand (0x28); /* Khoi tao giao tiep 4bit, 2 dong ky tu, font 5x8  */
	LCD_sendCommand (0x06);	/* Khoi tao huong di chuyen cua con tro sang phai */
	LCD_Backlight(1); /* Bat den nen */
	LCD_DisAndCurControl(1,0,0); /* Display ON, Cursor OFF */	
	LCD_clear(); /* Clear display */
	LCD_setCursor(0,0); /* Dua con tro ve hang 1 cot 1 */
}

	