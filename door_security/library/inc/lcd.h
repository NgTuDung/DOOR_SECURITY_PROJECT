#ifndef LCD_H
#define LCD_H

#include <util/delay.h>
#include <stdbool.h>

#define LCD_I2C_ADDRESS 0x27  // Địa chỉ I2C của LCD, A0,A1,A2 được đưa lên 1
#define LCD_RS 0x01  // P0 trên PCF8574
#define LCD_RW 0x02  // P1 trên PCF8574
#define LCD_EN 0x04  // P2 trên PCF8574

#define DISPLAY 1U
#define CURSOR 0U
#define LEFT  0U
#define RIGHT 1U

void LCD_sendCommand(uint8_t cmd);
void LCD_sendData(uint8_t data);

void LCD_init();	//Sử dụng hàm này để khởi tạo
void LCD_print(char *str);	//Sử dụng hàm này để in dữ liệu lên LCD
void LCD_clear();	//Sử dụng hàm này để xóa hết các thứ đang có trên LCD
void LCD_DisAndCurControl(bool a, bool b, bool c); // Hàm này Set chế độ cho hiển thị ký tự, con trỏ, nhấp nháy con trỏ
void LCD_Shift(bool a, bool b); //Hàm này set dịch chuyển cho toàn bộ ký tự sang phải hoặc trái 1 ô 
void LCD_Backlight(bool a); // Hàm này set trạng thái bật hoặc tắt đèn nền lcd
void LCD_setCursor(uint8_t col, uint8_t row);	//Hàm này để đưa con trỏ trên LCD đến 1 vị trí cụ thể với 2 tham số đầu vào là CỘT 'col' và HÀNG 'row'
#endif