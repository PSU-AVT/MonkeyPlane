#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "uart.h"

#define BAUD_RATE 38400

#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))

void pwm_init(void) {
    // Using Channel A on PC6 set COM1A1 --> Clear OCnA/OCnB/OCnC on compare match,set OCnA/OCnB/OCnC at TOP
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR3A = (1 << COM3A1) | (1 << WGM31);

    // WGM11, WGM12 and WGM13 set PWM output mode 14 so ICR1 is top
    TCCR1B = (1 << WGM12) | (1 << WGM13) | (1 << CS11); // <--- Prescaling by 8
    TCCR3B = (1 << WGM32) | (1 << WGM33) | (1 << CS31); // <--- Prescaling by 8

    // WGMn Bits must be set before ICR1
    // Using ICR1 as TOP For 50Hz Output at max resolution set at 40,000
    // 16,000,000 / 8 * 40001 = ~50hz
    ICR1 = 40000;
    ICR3 = 40000;

    // PB5 and PB6
    DDRB |= (1 << PB5) | (1 << PB6);

    // PC6
    DDRC |= (1 << PC6);

    OCR1A = 3400;
    OCR1B = 3400;
    OCR3A = 3400;
} 

// write a string to the uart
#define uart_print(s) uart_print_P(PSTR(s))
void uart_print_P(const char *str)
{
	char c;
	while (1) {
		c = pgm_read_byte(str++);
		if (!c) break;
		uart_putchar(c);
	}
}

void uart_get_pwms(uint8_t *pwms)
{
    char c;
    while((c = uart_getchar()));
    pwms[0] = uart_getchar();
    pwms[1] = uart_getchar();
    pwms[2] = uart_getchar();
}

#define PROTO_INTERCEPT 2199
#define PROTO_SCALE 6
#define PROTO_VAL_TO_OCR(x) ((x + PROTO_INTERCEPT) * PROTO_SCALE)

void set_signals(uint8_t *pwms)
{
    OCR1A = pwms[0] ? PROTO_VAL_TO_OCR(pwms[0]) : 2200;   
    OCR1B = pwms[1] ? PROTO_VAL_TO_OCR(pwms[1]) : 3000;   
    OCR3A = pwms[2] ? PROTO_VAL_TO_OCR(pwms[2]) : 3000;   
}

// A very basic example...
// when the user types a character, print it back
int main(void)
{
    uint8_t signals[3] = {1, 1, 1};

	CPU_PRESCALE(0);  // run at 16 MHz
    pwm_init();
	uart_init(BAUD_RATE);
	uart_print("UART Example\r\n");
	while (1) {
		if (uart_available()) {
            uart_get_pwms(signals);
            set_signals(signals);
		}
	}
}
