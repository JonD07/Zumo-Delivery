/*
         MEGN540 Mechatronics Lab
    Copyright (C) Andrew Petruska, 2021.
       apetruska [at] mines [dot] edu
          www.mechanical.mines.edu
*/

/*
    Copyright (c) 2021 Andrew Petruska at Colorado School of Mines

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

#include "Timing.h"


/** These define the internal counters that will be updated in the ISR to keep track of the time
 *  The volatile keyword is because they are changing in an ISR, the static means they are not
 *  visible (not global) outside of this file.
 */
static volatile uint32_t _count_ms = 0;

/**
 * Function SetupTimer0 initializes Timer0 to have a prescalar of XX and initializes the compare
 * feature for use in an ISR.  It also enables ISR's.
 *
 *  FCPU = 16 000 000 Hz,  1 us => 1 000 000 Hz,  A prescalar of 16 would give you millisecond resolution, but you dont
 *  have enough time to do those additions every microsecond, in addition the 8-bit timer will roll over every 256 microseconds.
 *  So, you need to be counting in (effectively) 4us increments.  So use a prescalar of 64. This will give you a resolution of 4us per
 *  Timer0 Bit and enough head-room to compare at with 249 for a 1kHz update.
 *
 *  Since Timer 0 will be triggering at a kHz, we may want other things to be updated at 1kHz too.
 *
 */
void SetupTimer0()
{
	// Set prescalor to 64
	TCCR0B |= (1 << CS00);
	TCCR0B |= (1 << CS01);

    // Set interrupt option, compare B
    TIMSK0 |= (1 << OCIE0B);

	// Set B compare value
	OCR0B = 249;

	// Enable ISR
	sei();

	// Init timer0 counter
	TCNT0 = 0;

    _count_ms= 0;
    ms_counter_1 = 0;
    ms_counter_2 = 0;
    ms_counter_3 = 0;
    ms_counter_4 = 0;
}

/**
 * This function gets the current time and returns it in a Time_t structure.
 */
Time_t GetTime()
{
	// Create and return Time_t struct with current time
	Time_t time = {
			.millisec = _count_ms,
			.microsec = TCNT0 * 64 * 0.0625
	};

	return time;
}

/**
 * This function gets the current time and returns it as a float (in seconds)
 */
float  GetTimeSec()
{
	// Grab current time and compute seconds
	Time_t time = GetTime();
	float time_seconds = 0.0;
	time_seconds += (float)time.millisec / 1000.0;
	time_seconds += (float)time.microsec / 1000000.0;
	return time_seconds;
}

/**
 * These functions return the individual parts of the Time_t struct, useful if you only care about
 * things on second or millisecond resolution.
 */
uint32_t GetMilli()
{
    return GetTime().millisec;
}

uint16_t GetMicro()
{
    return GetTime().microsec;
}


/**
 * This function takes a start time and calculates the time since that time, it returns it in the Time struct.
 * @param p_time_start a pointer to a start time struct
 * @return (Time_t) Time since the other time.
 */
Time_t SecondsSince(const Time_t* time_start_p )
{
	Time_t delta_time, current_time;

	// Grab current time
	current_time = GetTime();
	// Determine time change
	delta_time.millisec = current_time.millisec - time_start_p->millisec;
	delta_time.microsec = current_time.microsec - time_start_p->microsec;

	return delta_time;
}

/*
 * This function runs a delay in microseconds. The logic for this function
 * is based off of wiring.c in the open-source Arduino-core library.
 *
 * AVOID USING THIS FOR LONG PERIOD OF TIME!!!
 */
void DelayMicroseconds(uint16_t us) {
	// Don't waste your time for 1 us. Function overhead is long enough
	// to last 1 us
	if (us <= 1) return;
	int j = 0;

	// The loop below takes 1/4 of a microsecond per iteration, so
	// quadruple the count.
	us <<= 2;

	// We have already wasted roughly 5 us (including this subtraction)...
	us -= 5;

	// Assembly busy-wait!
//	__asm__ __volatile__ (
//		"1: sbiw %0,1" "\n\t"				// 2 cycles
//		"brne 1b" : "=w" (us) : "0" (us)	// 2 cycles
//	);
	for(int i = 0; i < us; i++) {
		j += 1;
	}
	j += 1;
}

/**
 * Interrupt Service Routine for the Timer0 Compare A feature.
 */
ISR(TIMER0_COMPB_vect)
{
	// Upticks of both our internal and external variables.
	_count_ms ++;

	// Reset timer
	TCNT0 = 0;

	ms_counter_1 ++;
	ms_counter_2 ++;
	ms_counter_3 ++;
	ms_counter_4 ++;
}
