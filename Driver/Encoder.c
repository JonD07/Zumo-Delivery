#include "Encoder.h"

/**
* Internal counters for the Interrupts to increment or decrement as necessary.
*/
static volatile bool _last_right_A;  // Static limits it's use to this file
static volatile bool _last_right_B;  // Static limits it's use to this file
static volatile bool _last_right_XOR; // Necessary to check if PE6 triggered the ISR or not

static volatile bool _last_left_A;   // Static limits it's use to this file
static volatile bool _last_left_B;   // Static limits it's use to this file
static volatile bool _last_left_XOR; // Necessary to check if PB4 triggered the ISR or not

static volatile int32_t _left_counts;   // Static limits it's use to this file
static volatile int32_t _right_counts;  // Static limits it's use to this file

/** Helper Funcions for Accessing Bit Information */
// *** MEGN540 Lab 3 TODO ***
// Hint, use avr's bit_is_set function to help
static inline bool Right_XOR() { return bit_is_set(PINE, 6); } // MEGN540 Lab 3 TODO
static inline bool Right_B()   { return bit_is_set(PINF, 0); } // MEGN540 Lab 3 TODO
static inline bool Right_A()   { return bit_is_set(PINE, 6) ^ bit_is_set(PINF, 0); } // MEGN540 Lab 3 TODO

static inline bool Left_XOR() { return bit_is_set(PINB, 4); } // MEGN540 Lab 3 TODO
static inline bool Left_B()   { return bit_is_set(PINE, 2); } // MEGN540 Lab 3 TODO
static inline bool Left_A()   { return bit_is_set(PINB, 4) ^ bit_is_set(PINE, 2); } // MEGN540 Lab 3 TODO

#define PI 3.14159

/**
 * Function Encoders_Init initializes the encoders, sets up the pin change interrupts, and zeros the initial encoder
 * counts.
 */
void Encoders_Init()
{
	// Enable interrupt for PCINT4 on PB4 pin change
	PCICR |= 0x01;
	PCMSK0 |= 0x10;
	// Enable interrupt for PCINT6 on PE6 pin change
	EIMSK |= 0x40;
	EICRB |= 0x10;

	// Initialize static file variables. These probably need to be updated.
	_last_left_B = bit_is_set(PINE, 2);
	_last_left_XOR = bit_is_set(PINB, 4);
	_last_left_A = (_last_left_B ^ _last_left_XOR);

	_last_right_B = bit_is_set(PINF, 0);
	_last_right_XOR = bit_is_set(PINE, 6);
	_last_right_A = (_last_right_B ^ _last_right_XOR);

	_left_counts = 0;
	_right_counts = 0;
}


/**
 * Function Counts_Left returns the number of counts from the left encoder.
 * @return [int32_t] The count number.
 */
int32_t Counts_Left()
{
	// Record global interrupt settings
	uint8_t sreg_value = SREG;
	// Disable interrupts
	cli();
	// Access encoder counts
	int32_t counts_left = _left_counts;
	// Restore global interrupt settings
	SREG = sreg_value;

	return counts_left;
}

/**
 * Function Counts_Right returns the number of counts from the right encoder.
 * @return [int32_t] The count number.
 */
int32_t Counts_Right()
{
	// Record global interrupt settings
	uint8_t sreg_value = SREG;
	// Disable interrupts
	cli();
	// Access encoder counts
	int32_t counts_right = _right_counts;
	// Restore global interrupt settings
	SREG = sreg_value;

	return counts_right;
}

void Zero_Encoders() {
	// Record global interrupt settings
	uint8_t sreg_value = SREG;
	// Disable interrupts
	cli();
	// Zero both encoders
	_right_counts = 0;
	_left_counts = 0;
	// Restore global interrupt settings
	SREG = sreg_value;
}

/**
 * Function Rad_Left returns the number of radians for the left encoder.
 * @return [float] Encoder angle in radians
 */
float Rad_Left()
{
	static float countsPerRevMotorShaftL = 12;
	static float gearRatioL = 75.81;
	float countsPerRevSprocketL;
	countsPerRevSprocketL = (countsPerRevMotorShaftL * gearRatioL);
	float radSprocketPerCountL;
	radSprocketPerCountL = 2*PI / (countsPerRevSprocketL);
	float numRadL;
	numRadL = (Counts_Left()) * radSprocketPerCountL;

	return numRadL;
}

/**
 * Function Rad_Right returns the number of radians for the left encoder.
 * @return [float] Encoder angle in radians
 */
float Rad_Right()
{
	static float countsPerRevMotorShaftR = 12;
	static float gearRatioR = 75.81;
	float countsPerRevSprocketR;
	countsPerRevSprocketR = (countsPerRevMotorShaftR * gearRatioR);
	float radSprocketPerCountR;
	radSprocketPerCountR = 2*PI / (countsPerRevSprocketR);
	float numRadR;
	numRadR = (Counts_Right()) * radSprocketPerCountR;

	return numRadR;
}

/**
 * Interrupt Service Routine for the left Encoder. Note: May need to check that it is actually PCINT4 that triggered, as
 * the Pin Change Interrupts can trigger for multiple pins.
 * @return
 */
ISR(PCINT0_vect)
{
	if(_last_left_XOR != Left_XOR()){
		_left_counts += (_last_left_B ^ Left_A()) - (_last_left_A ^ Left_B());
		_last_left_A = Left_A();
		_last_left_B = Left_B();
		_last_left_XOR = Left_XOR();
	}
}


/**
 * Interrupt Service Routine for the right Encoder.
 * @return
 */
ISR(INT6_vect)
{
	if(_last_right_XOR != Right_XOR()){
		_right_counts += (_last_right_B ^ Right_A()) - (_last_right_A ^ Right_B());
		_last_right_A = Right_A();
		_last_right_B = Right_B();
		_last_right_XOR = Right_XOR();
	}
}
