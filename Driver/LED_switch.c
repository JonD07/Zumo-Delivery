#include "LED_switch.h"

/*
 * LED_Init initializes the LED lights to turn on and off. We are
 * using pins PD5 for RED, PD0 for GREEN, and PD3 for BLUE
 */
void LED_Init() {
	// Init PD5: Set pin to write, set output low
	DDRD |= (1 << DDD5);
	PORTD &= ~(1 << PORTD5);

	// Init PD0
	DDRD |= (1 << DDD0);
	PORTD &= ~(1 << PORTD0);

	// Init PD3
	DDRD |= (1 << DDD3);
	PORTD &= ~(1 << PORTD3);
}

/*
 * Set_LED turns LED led on or off based on boolean input on.
 */
void Set_LED(eLEDLights led, bool on) {
	switch(led) {
	case RED:
		if(on) {
			// Turn on red LED
			PORTD |= (1 << PORTD5);
		}
		else {
			// Turn off red LED
			PORTD &= ~(1 << PORTD5);
		}

		break;

	case GREEN:
		if(on) {
			PORTD |= (1 << PORTD0);
		}
		else {
			PORTD &= ~(1 << PORTD0);
		}

		break;

	case BLUE:
		if(on) {
			PORTD |= (1 << PORTD3);
		}
		else {
			PORTD &= ~(1 << PORTD3);
		}

		break;
	}
}
