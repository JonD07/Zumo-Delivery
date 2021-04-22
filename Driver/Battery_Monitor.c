#include "Battery_Monitor.h"

static const float BITS_TO_BATTERY_VOLTS = 0.004887585533;

static Filter_Data_t battery_filter;
static float a_coeff[] = {1.0, -1.9112, 0.9150};
static float b_coeff[] = {9.4469E-4, 18.8938E-4, 9.4469E-4};
static int order;

/**
 * Function Battery_Monitor_Init initializes the Battery Monitor to record the current battery voltages.
 */
void Battery_Monitor_Init()
{
	// Set reference voltage to internal 2.56V and set channel to ADC6
	ADMUX = 0xC6;
	// Enable ADC and set prescalor to 128
	ADCSRA |= 0x87;

	// Initialize the filter
	order = sizeof(a_coeff)/sizeof(float) -1;
	Filter_Init(&battery_filter, b_coeff, a_coeff, order);
    Filter_SetTo(&battery_filter, Battery_Voltage_Oneshot());
}

/**
 * Returns single battery voltage sample
 */
float Battery_Voltage_Oneshot()
{
	// Set channel to ADC6
	ADMUX = ((ADMUX & 0xE0) | 0x06);
	// Trigger ADC conversion
	ADCSRA |= 0x40;
	// Wait for conversion to finish
	while(ADCSRA & 0x40)
		__asm(" nop");

	return (float)ADC * BITS_TO_BATTERY_VOLTS;
}

/**
 * Runs the battery upkeep task, return most recent filtered reading
 */
float Battery_Voltage_Task()
{
	return Filter_Value(&battery_filter, Battery_Voltage_Oneshot());
}

/**
 * Function Battery_Voltage initiates the A/D measurement and returns the result for the battery voltage.
 */
float Battery_Voltage()
{
	return Filter_Last_Output(&battery_filter);
}
