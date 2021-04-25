#include "MotorPWM.h"

/**
 * Function MotorPWM_Init initializes the motor PWM on Timer 1 for PWM based voltage control of the motors.
 * The Motor PWM system shall initialize in the disabled state for safety reasons. You should specifically enable
 * Motor PWM outputs only as necessary.
 * @param [uint16_t] MAX_PWM is the maximum PWM value to use. This controls the PWM frequency.
 */
void Motor_PWM_Init( uint16_t MAX_PWM )
{
	/* Enable waveform generation mode 10, set register TCCR1B, fields WGM13 to 1 and WGM12
	to 0; and set register TCCR1A, fields WGM11 and WGM10 to 0*/
	//Note: WGM12, WGM11, and WGM10 are all off by default
	TCCR1B = 0x11;
	TCCR1A &= 0b11111110;
	TCCR1A |= 0b00000010;
	// Disable the output, set register TCCR1A, fields COM1A1 and COM1A0 to 0
	TCCR1A &= 0b00001111;
	// Set motor output pins to write
	DDRB |= (1 << DDB1) | (1 << DDB2) | (1 << DDB5) | (1 << DDB6);
	// Disable interrupts
	cli();
	//Set the TOP value (ICR1) equal to MAX_PWM
	ICR1 = MAX_PWM;
	// Enable interrupts
	sei();
}

/**
 * Function MotorPWM_Enable enables or disables the motor PWM outputs.
 * @param [bool] enable (true set enable, false set disable)
 */
void Motor_PWM_Enable( bool enable )
{
	if(enable==true)
	{
		TCCR1A |= 0b11110000;
	}
	else
	{
		TCCR1A &= 0b00001111;
	}
}

/**
 * Function Is_Motor_PWM_Enabled returns if the motor PWM is enabled for output.
 * @param [bool] true if enabled, false if disabled
 */
bool Is_Motor_PWM_Enabled()
{
	if((TCCR1A&(1<<COM1A1)) && (TCCR1A&(1<<COM1A0)))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Function Motor_PWM_Left sets the PWM duty cycle for the left motor.
 * @input int16_t pwm: the % duty cycle
 */
void Motor_PWM_Left( int16_t pwm )
{
	// Set motor direction pins
	if(pwm < 0)
	{
		PORTB |= 0b00000100;
		pwm = pwm * -1;
	}
	else
	{
		PORTB &= 0b11111011;
	}

	pwm = (pwm > 100) ? 100 : pwm;
	// Disable interrupts
	cli();
	OCR1B = (int16_t)((((float)pwm * -0.01) + 1) * ICR1);
	// Enable interrupts
	sei();
}

/**
 * Function Motor_PWM_Right sets the PWM duty cycle for the right motor.
 * @input int16_t pwm: the % duty cycle
 */
void Motor_PWM_Right( int16_t pwm )
{
	// Set motor direction pins
	if(pwm < 0)
	{
		PORTB |= 0b00000010;
		pwm = pwm * -1;
	}
	else
	{
		PORTB &= 0b11111101;
	}


	pwm = (pwm > 100) ? 100 : pwm;
	// Disable interrupts
	cli();
	OCR1A = (int16_t)((((float)pwm * -0.01) + 1) * ICR1);
	// Enable interrupts
	sei();
}

/**
 * Function Get_Motor_PWM_Left returns the current PWM duty cycle for the left motor. If disabled it returns what the
 * PWM duty cycle would be.
 * @return [int16_t] duty-cycle for the left motor's pwm
 */
int16_t Get_Motor_PWM_Left()
{
	return -100*(OCR1B / (float)Get_MAX_Motor_PWM()) + 100;
}

/**
 * Function Get_Motor_PWM_Right returns the current PWM duty cycle for the right motor. If disabled it returns what the
 * PWM duty cycle would be.
 * @return [int16_t] duty-cycle for the right motor's pwm
 */
int16_t Get_Motor_PWM_Right()
{
	return -100*(OCR1A / (float)Get_MAX_Motor_PWM()) + 100;
}

/**
 * Function Get_MAX_Motor_PWM() returns the PWM count that corresponds to 100 percent duty cycle (all on), this is the
 * same as the value written into ICR1 as (TOP).
 */
uint16_t Get_MAX_Motor_PWM()
{
	return ICR1;
}

/**
 * Function Set_MAX_Motor_PWM sets the maximum pwm count. This function sets the timer counts to zero because
 * the ICR1 can cause undesired behaviors if change dynamically below the current counts.  See page 128 of the
 * atmega32U4 datasheat.
 */
void Set_MAX_Motor_PWM( uint16_t MAX_PWM )
{
	// Disable interrupts
	cli();
	//Set the TOP value (ICR1) equal to MAX_PWM
	ICR1 = MAX_PWM;
	// Enable interrupts
	sei();
}

float DutyCycle_to_Velocity_Left(int duty_cycle) {
	return ((float)duty_cycle * 0.0033) + 0.0133;
}

float DutyCycle_to_Velocity_Right(int duty_cycle) {
	return ((float)duty_cycle * 0.0034) + 0.0133;
}

int Velocity_to_DutyCycle_Left(float velocity) {
	return (int)((velocity - 0.0133)/0.0033);
}

int Velocity_to_DutyCycle_Right(float velocity) {
	return (int)((velocity - 0.0133)/0.0034);
}

float ECount_to_Distance(int encoder_count) {
	return ((float)encoder_count * 2 * PI * 0.0195) / (12 * 75.81);
}
