#include "ServoPWM.h"

/**
 * Function Servo_PWM_Init initializes a PWM signal on Timer 4 for the gripper
 * servo motor.
 *
 * TODO: For safety (and the sake of not having fun), make sure that this is
 * initialized to the open position
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

	// Set compare value
	uint8_t duty_cycle = DUTY_CYCLE_MAX;
	switch(state) {
	case OPEN:
		duty_cycle = DUTY_CYCLE_MIN;
		break;
	case M25:
		duty_cycle = (DUTY_CYCLE_MAX - DUTY_CYCLE_MIN)/4 + DUTY_CYCLE_MIN;
		break;
	case M50:
		duty_cycle = (DUTY_CYCLE_MAX - DUTY_CYCLE_MIN)/2 + DUTY_CYCLE_MIN;
		break;
	case M75:
		duty_cycle = (DUTY_CYCLE_MAX - DUTY_CYCLE_MIN)*3/4 + DUTY_CYCLE_MIN;
		break;
	case CLOSE:
		duty_cycle = DUTY_CYCLE_MAX;
		break;
	}
//	OCR4A = (state == OPEN) ? DUTY_CYCLE_MIN : DUTY_CYCLE_MAX;
	OCR4A = duty_cycle;

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
}

/**
 * Function Servo_Is_Closed returns true if the gripper is closed
 */
bool Servo_Is_Closed()
{
	return (OCR4A < (DUTY_CYCLE_MIN + 10));
}
