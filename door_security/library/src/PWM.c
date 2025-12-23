#include <avr/io.h>

#define F_CPU 1000000UL // Tần số CPU 1 MHz

void PWM_init() {
    // Cấu hình Timer1 ở chế độ Fast PWM, Gia tri top bang ICR1
    TCCR1A|=(1<<COM1A1)|(1<<WGM11); //NON Inverted PWM

    TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS10); //Khong prescaler, xung clock 1MHz

    ICR1 = 19999;  // Đặt TOP = 20ms (tần số 50Hz)
    DDRD |= (1 << PD5); // Đặt chân OC1A (PD5) làm output
}