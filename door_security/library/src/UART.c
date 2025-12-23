/*
 * UART.c
 *
 * Created: 4/16/2025 4:57:08 PM
 *  Author: FPTSHOP
 */
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "UART.h"

#define F_CPU 1000000UL // T?n s? CPU 1 MHz

volatile char uart_buffer[UART_BUFFER_SIZE];
volatile uint8_t uart_index = 0;
volatile bool UART_flag = false;
volatile char UART_data = 0;

void UART_init() {
	// bat che do double speed
	UCSRA |= (1 << U2X);
	
	// Set Baudrate
	uint16_t ubrr = MYUBRR;
	UBRRH = (uint8_t)(ubrr >> 8);
	UBRRL = (uint8_t)ubrr;

	// enable chan RX va TX, cho phep ngat RX
	UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);

	// Che do khong dong bo, khong parity bit, frame truyen 8 bit, 1 stop bit.
	UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);
}

ISR(USART_RXC_vect) {
	char received = UDR;

	if (!UART_flag) {
		if (received == ',') {
			uart_buffer[uart_index] = '\0'; // Ket thuc chuoi
			UART_flag = true;              // Danh dau de co ngat la da nhan xong va xu ly chuoi
			uart_index = 0;
		} else {
			if (uart_index < UART_BUFFER_SIZE - 1) {
				uart_buffer[uart_index++] = received;
			} else {
				uart_index = 0; // Reset neu gui chuoi qua dai
			}
		}
	}

}

void UART_sendChar(char data)
{
	while (!(UCSRA & (1 << UDRE))); //Doi den khi thanh ghi du lieu UDR trong
	UDR = data;
}

void UART_sendString(const char* str)
{
	while (*str)
	{
		UART_sendChar(*str++);
	}
}
