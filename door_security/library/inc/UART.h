/*
 * IncFile1.h
 *
 * Created: 4/16/2025 4:55:41 PM
 *  Author: FPTSHOP
 */ 


#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define UART_BUFFER_SIZE 32

extern volatile char uart_buffer[UART_BUFFER_SIZE];
extern volatile bool UART_flag;

#define BAUDRATE 2400
#define MYUBRR ((F_CPU / (8UL * BAUDRATE)) - 1)

void UART_init();
void UART_sendChar(char data);
void UART_sendString(const char* str);

#endif /* UART_H_ */