#include "i2c.h" // Include thư viện I2C đã viết
#include "lcd.h"



void LCD_sendCommand(uint8_t cmd) {
	uint8_t highNibble = (cmd & 0xF0); // RS=0
	uint8_t lowNibble = ((cmd << 4) & 0xF0); //RS=0
	
	// Truyền byte cao
	I2C_start();
	I2C_write(LCD_I2C_ADDRESS);
	I2C_write(highNibble | LCD_EN);
	I2C_write(highNibble);
	I2C_stop();
	
	// Truyền byte thấp
	I2C_start();
	I2C_write(LCD_I2C_ADDRESS);
	I2C_write(lowNibble | LCD_EN);
	I2C_write(lowNibble);
	I2C_stop();
}

void LCD_sendData(uint8_t data) {
	int8_t highNibble = (data & 0xF0) | LCD_RS | 0x08; //RS=1
	uint8_t lowNibble = ((data << 4) & 0xF0) | LCD_RS | 0x08; //RS=1
	
	// Truyền byte cao
	I2C_start();
	I2C_write(LCD_I2C_ADDRESS);
	I2C_write(highNibble | LCD_EN);
	I2C_write(highNibble);
	I2C_stop();
	
	// Truyền byte thấp
	I2C_start();
	I2C_write(LCD_I2C_ADDRESS);
	I2C_write(lowNibble | LCD_EN);
	I2C_write(lowNibble);
	I2C_stop();
}

void LCD_init() {
	
	_delay_ms(20);  /* LCD Power ON Initialization time >15ms */
	LCD_sendCommand (0x02); /* 4bit mode */
	LCD_sendCommand (0x28); /* Initialization of 16X2 LCD in 4bit mode */
	LCD_sendCommand (0x0C); /* Display ON Cursor OFF */
	LCD_sendCommand (0x06); /* Auto Increment cursor */
	LCD_sendCommand (0x01); /* Clear display */
	LCD_sendCommand (0x80); /* Cursor at home position */
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