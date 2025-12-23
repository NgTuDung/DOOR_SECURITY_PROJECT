#ifndef I2C_H
#define I2C_H

#include <avr/io.h>

#define F_CPU 1000000UL  // Đặt tần số cho vi điều khiển
#define SCL_CLOCK 100000L  // Đặt tốc độ xung cho I2C là 100kHz

void I2C_init(void);
void I2C_start(void);
void I2C_stop(void);
void I2C_write(uint8_t data);

#endif