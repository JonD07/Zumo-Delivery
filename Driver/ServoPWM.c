#include "ServoPWM.h"

/**
 * Function Servo_PWM_Init initializes the Servo PWM on Timer 4 for PWM based voltage control of the servo.
 * The Servo PWM system shall initialize in the disabled state for safety reasons. You should specifically enable
 * Servo PWM outputs only as necessary.
 * @param [uint16_t] MAX_PWM is the maximum PWM value to use. This controls the PWM frequency.
 */
void Servo_PWM_Init()
{
	// Set fast-PWM, set at timer reset, clear on compare
	TCCR4A |= 0b10000010;
	TCCR4A &= ~(0b01000000);

	// Enable clock with prescaler of 2048
	TCCR4B &= ~(0x0F);
	TCCR4B |= (0x0C);

	// Set PWM fast-mode
	TCCR4D &= ~(0b00000011);

	// Set DDR pin to write for OC4A -> PC7
	DDRC |= (1 << DDC7);

	// Set compare value
	OCR4A = DUTY_CYCLE_MIN;

	// Configure timer
	uint8_t oldSREG = SREG;
	cli();
	// Set TOP value
	OCR4C = MAX_PWM;
	// Clear timer
	TCNT4 = 0;
	TC4H &= ~(0x07);
	SREG = oldSREG;
}

/**
 * Function Close_Servo sets the PWM duty cycle for the servo to close.
 */
void Close_Servo()
{
	// Disable interrupts
	cli();
	// Set compare value
	OCR4A = DUTY_CYCLE_MAX;
	// Enable interrupts
	sei();
}

/**
 * Function Open_Servo sets the PWM duty cycle for the servo to open.
 */
void Open_Servo()
{
	// Disable interrupts
	cli();
	// Set compare value
	OCR4A = DUTY_CYCLE_MIN;
	// Enable interrupts
	sei();
}

/**
 * Function Servo_Is_Closed returns true if the gripper is closed
 */
bool Servo_Is_Closed()
{
	return (OCR4A < (DUTY_CYCLE_MIN + 10));
}
