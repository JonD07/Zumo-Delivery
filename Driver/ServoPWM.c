#include "ServoPWM.h"

/**
 * Function Servo_PWM_Init initializes the Servo PWM on Timer 4 for PWM based voltage control of the servo.
 * The Servo PWM system shall initialize in the disabled state for safety reasons. You should specifically enable
 * Servo PWM outputs only as necessary.
 * @param [uint16_t] MAX_PWM is the maximum PWM value to use. This controls the PWM frequency.
 */
void Servo_PWM_Init( uint16_t MAX_PWM )
{
	/*
	Enable phase and frequency correct PWM waveform generation mode, set register WGM41 to 0, WGM40 to 1, PWM4A to 1
	*/
	TCCR4A &= 0b00001111; // Disable output, set COM4A1, COM4A0 to 0
	TCCR4A |= 0b00000010; // PWM4A = 1
	TCCR4D &= 0b11111101; // WGM41 = 0
	TCCR4D |= 0b00000001; // WGM40 = 1

	//Set DDR pins for OC4A -> PC7
	DDRC |= (1 << DDC7);

	cli(); // Disable Interrupts
	OCR4C = MAX_PWM; //Set TOP value
	sei(); // Enable Interrupts
}

/**
 * Function ServoPWM_Enable enables or disables the Servo PWM outputs.
 * @param [bool] enable (true set enable, false set disable)
 */
void Servo_PWM_Enable( bool enable )
{
	if(enable==true)
	{
		TCCR4A |= 0b11110000;
	}
	else
	{
		TCCR4A &= 0b00001111;
	}
}

/**
 * Function Is_Servo_PWM_Enabled returns if the Servo PWM is enabled for output.
 * @param [bool] true if enabled, false if disabled
 */
bool Is_Servo_PWM_Enabled()
{
	if((TCCR4A&(1<<COM4A1)) && (TCCR4A&(1<<COM4A0)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Function Servo_PWM sets the PWM duty cycle for the servo to close.
 */
void Close_Servo()
{

	// pwm = (pwm > 100) ? 100 : pwm;
	// // Disable interrupts
	// cli();
	// OCR1A = (int16_t)((((float)pwm * -0.01) + 1) * ICR1);
	// // Enable interrupts
	// sei();
}

/**
 * Function Servo_PWM sets the PWM duty cycle for the servo to open.
 */
void Open_Servo()
{

	// pwm = (pwm > 100) ? 100 : pwm;
	// // Disable interrupts
	// cli();
	// OCR1A = (int16_t)((((float)pwm * -0.01) + 1) * ICR1);
	// // Enable interrupts
	// sei();
}

/**
 * Function Get_Servo_PWM returns the current PWM duty cycle for the Servo. If disabled it returns what the
 * PWM duty cycle would be.
 * @return [int16_t] duty-cycle for the left Servo's pwm
 */
int16_t Get_Servo_PWM()
{
	return -100*(OCR1B / (float)Get_MAX_Servo_PWM()) + 100;
}

/**
 * Function Get_MAX_Servo_PWM() returns the PWM count that corresponds to 100 percent duty cycle (all on), this is the
 * same as the value written into ICR1 as (TOP).
 */
uint16_t Get_MAX_Servo_PWM()
{
	return ICR1;
}

/**
 * Function Set_MAX_Servo_PWM sets the maximum pwm count. This function sets the timer counts to zero because
 * the ICR1 can cause undesired behaviors if change dynamically below the current counts.  See page 128 of the
 * atmega32U4 datasheat.
 */
void Set_MAX_Servo_PWM( uint16_t MAX_PWM )
{
	// Disable interrupts
	cli();
	//Set the TOP value (ICR1) equal to MAX_PWM
	ICR1 = MAX_PWM;
	// Enable interrupts
	sei();
}
