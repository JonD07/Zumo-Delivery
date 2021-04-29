#include "ServoPWM.h"

/**
 * Function Servo_PWM_Init initializes a PWM signal on Timer 4 for the gripper
 * servo motor.
 */
void Servo_PWM_Init(eGripperState state)
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

	if(state == OPEN) {
		// Set compare value
		OCR4A = DUTY_CYCLE_OPEN;
		Set_LEDs(OPEN);
	}
	else {
		// Set compare value
		OCR4A = DUTY_CYCLE_CLOSED;
		Set_LEDs(CLOSE);
	}

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
	// Configure timer
	uint8_t oldSREG = SREG;
	cli();
	// Set compare value
	OCR4A = DUTY_CYCLE_MAX;
	SREG = oldSREG;

	// Set LEDs
	Set_LEDs(CLOSE);
}

/**
 * Function Open_Servo sets the PWM duty cycle for the servo to open.
 */
void Open_Servo()
{
	// Configure timer
	uint8_t oldSREG = SREG;
	cli();
	// Set compare value
	OCR4A = DUTY_CYCLE_MIN;
	SREG = oldSREG;

	// Set LEDs
	Set_LEDs(OPEN);
}

/**
 * Function Servo_Is_Closed returns true if the gripper is closed
 */
bool Servo_Is_Closed()
{
	return (OCR4A < (DUTY_CYCLE_MIN + 10));
}

/*
 * Set_LEDs configures the LEDs based on OPEN/CLOSED states
 */
void Set_LEDs(eGripperState state) {
	if(state == OPEN) {
		// Turn on green, turn off blue
		Set_LED(GREEN, true);
		Set_LED(BLUE, false);
	}
	else if(state == CLOSE) {
		// Turn on blue, turn off green
		Set_LED(BLUE, true);
		Set_LED(GREEN, false);
	}
	else {
		// Thing went wrong.. turn everything off
		Set_LED(GREEN, false);
		Set_LED(BLUE, false);
	}
}
