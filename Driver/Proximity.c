/*
    Copyright (c) 2021 Jonathan Diller at Colorado School of Mines

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/
#include "Proximity.h"

/*
 * Static variables
 */
/// Hardware related variables
// This gives the strobe a period of 38 kHz
static const uint16_t stobe_period = 420;
// The sensor docs recommend waiting 7 - 15 cycles. Wait 16/(38 kHz) = 421 us
static const uint16_t pulseOnTimeUs = 421;
// Need to wait 6 cycles to read after turning on. 22/(38 kHz) = 578 us
static const uint16_t pulseOffTimeUs = 578;

static uint16_t m_nLEDProxyCountLeft;
static uint16_t m_nLEDProxyCountRight;

// Brightness levels
static uint16_t levelsArray[] = { 4, 15 };
// This needs to reflect the size of the above array!
static uint8_t numLevels = 2;


/*
 * Initializes the front facing IR proximity sensor
 */
void Proxy_Init() {
	m_nLEDProxyCountLeft = 0;
	m_nLEDProxyCountRight = 0;
}

/*
 * Runs the IR read logic
 */
void IRRead(eProximitySize side) {
	// Set read pin
	uint8_t oldSREG = SREG;
	cli();
	// Set input with pullup
	DDRF &= ~(0x01);
	PINF |= 0x01;
	PORTF |= 0x01;
	// Ensure line sensor lights are off
	PORTB &= ~(0x80);
	DDRB &= ~(0x80);
	SREG = oldSREG;
	// Wait before continuing
	DelayMicroseconds(pulseOffTimeUs);

	// Reset the IR readings
	if(side == LEFT) {
		m_nLEDProxyCountLeft = 0;
	}
	else if(side == RIGHT) {
		m_nLEDProxyCountRight = 0;
	}

	for(int i = 0; i < numLevels; i++) {
		// Start the IR strobe and wait
		start_strobe(side, levelsArray[i]);
		// Give sensor some time before checking input
		DelayMicroseconds(pulseOnTimeUs);
		// Record result. If the pin is low, then we have a hit at this brightness
		if(!bit_is_set(PINF, 1)) {
			if(side == LEFT) {
				m_nLEDProxyCountLeft++;
			}
			else if(side == RIGHT) {
				m_nLEDProxyCountRight++;
			}
		}
		// Shut-off strobe
		stop_strobe();
		// Wait before continuing to next strobe
		DelayMicroseconds(pulseOffTimeUs);
	}
}

/*
 * Returns max( left-LED-counts, right-LED-counts)
 */
t_ProximityReturn IR_Counts() {
	t_ProximityReturn ret_val;

	if(m_nLEDProxyCountLeft >= m_nLEDProxyCountRight) {
		// Return the left hand reading
		ret_val.m_nCount = m_nLEDProxyCountLeft;
		ret_val.m_eSide = LEFT;
	}
	else {
		// Return the right hand reading
		ret_val.m_nCount = m_nLEDProxyCountRight;
		ret_val.m_eSide = RIGHT;
	}

	return ret_val;
}

/*
 * This function starts the IR LED strobe based on the given
 * brightness level and set period.
 */
void start_strobe(eProximitySize side, uint16_t brightness) {
	// Disable Timer 3's interrupts.
	TIMSK3 = 0;

	// Set the PWM pin to be an input temporarily. This is needed
	// to avoid errors on the output.
	PORTC &= ~(1 << 6);
	DDRC &= ~(1 << 6);

	// COM3A<1:0> = 10 : Clear OC3A on match, set at top.
	TCCR3A = (1 << COM3A1);
	TCCR3B = 0;

	// Drive OC3A signal low by simulating a match
	TCCR3C = (1 << FOC3A);

	// Make the PWM pin be an output. OC03A will control its value.
	DDRC |= (1 << 6);

	if(side == LEFT) {
		// Clear PORTF6
		PORTF &= ~(1 << 6);
	}
	else if(side == RIGHT) {
		// Set PORTF6
		PORTF |= (1 << 6);
	}

	// Set PORTF6 as an output
	DDRF |= (1 << 6);

	// Set frequency for compare 3
	ICR3 = stobe_period;

	// Set the count so that compare is triggered quickly
	TCNT3 = stobe_period - 1;

	// Set compare value (duty cycle).
	OCR3A = brightness;

	// Start the timer
	// COM3A<1:0> = 10 : Set OC3A on match, clear at top.
	// WGM3<3:0> = 1110 : Fast PWM, with ICR3 as the TOP.
	// CS3<3:0> = 001 : Internal clock with no prescaler
	TCCR3A = (1 << COM3A1) | (1 << WGM31);
	TCCR3B = (1 << WGM33) | (1 << WGM32) | (1 << CS30);
}

/*
 * This function stops the IR LED strobe. The logic for this function
 * is based off of wiring.c in the open-source Arduino-core library.
 */
void stop_strobe()
{
	// Drive PWM pin low. This avoids letting the pin float
	PORTC &= ~(1 << 6);
	DDRC |= (1 << 6);

	// Disconnect PWM and drive output low
	TCCR3A = (1 << WGM31);

	// Turn off timer.
	TCCR3B = 0;

	// Restore the timer's default settings
	TIMSK3 = 0;
	TCCR3A = 0;
	OCR3A = 0;
	ICR3 = 0;
	TCNT3 = 0;

	// Set IR LED direction pin back
	DDRF &= ~(1 << 6);
	PORTF &= ~(1 << 6);
}
