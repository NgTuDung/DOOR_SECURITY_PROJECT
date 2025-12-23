#include <avr/io.h>

#define F_CPU 1000000UL // Tần số CPU 1 MHz
#define SERVO_MIN 1000  // Độ rộng xung tối thiểu: 1 ms (0 độ)
#define SERVO_MAX 2000  // Độ rộng xung tối đa: 2 ms (180 độ)

void PWM_init() {
    // Cấu hình Timer1 ở chế độ Fast PWM, Gia tri top bang ICR1
    TCCR1A|=(1<<COM1A1)|(1<<WGM11); //NON Inverted PWM

    TCCR1B|=(1<<WGM13)|(1<<WGM12)|(1<<CS10); //Khong prescaler, xung clock 1MHz

    ICR1 = 19999;  // Đặt TOP = 20ms (tần số 50Hz), ICR1 = (F_CPU / (Prescaler * F_PWM)) - 1
    DDRD |= (1 << PD5); // Đặt chân OC1A (PD5) làm output
}