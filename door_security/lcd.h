#ifndef LCD_H
#define LCD_H

#include <util/delay.h>

#define LCD_I2C_ADDRESS 0x4E  // Địa chỉ I2C của LCD, có thể cần thay đổi tùy module
#define LCD_RS 0x01  // P0 trên PCF8574
#define LCD_RW 0x02  // P1 trên PCF8574
#define LCD_EN 0x04  // P2 trên PCF8574

void LCD_sendCommand(uint8_t cmd);
void LCD_sendData(uint8_t data);

void LCD_init();	//Sử dụng hàm này để khởi tạo
void LCD_print(char *str);	//Sử dụng hàm này để in dữ liệu lên LCD
void LCD_clear();	//Sử dụng hàm này để xóa hết các thứ đang có trên LCD 
void LCD_setCursor(uint8_t col, uint8_t row);	//Hàm này để đưa con trỏ trên LCD đến 1 vị trí cụ thể với 2 tham số đầu vào là CỘT 'col' và HÀNG 'row'
#endif