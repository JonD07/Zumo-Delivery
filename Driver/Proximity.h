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

/**
 * Proximity.h/c defines functions for the IR proximity sensor. We used the Zumo
 * 32U4 library to help design the functionality of the IR proximity sensor.
 *
 * The IR works be strobing an IR LED and then reading the digital IR sensor to
 * determine if the strobe was bright enough to reflect off of another surface.
 * As we increase the brightness, the father away of an object we can detect. The
 * returned distances are based off of this. The strobe should run at 38 kHz for the
 * sensor to register the strobing light. The brightness levels and period of the
 * strobe are set in the .c file. Make sure brightness is not larger than period
 * because then the compare match would never happen and the pulse count would
 * always be zero.
 */
#ifndef PROXIMITY_H
#define PROXIMITY_H

#include <avr/interrupt.h>
#include <stdbool.h>

#include "Timing.h"

typedef struct IRSensorData
{
	uint8_t m_CountLeftLED;
	uint8_t m_CountRightLED;
} IRSensorData;

/*
 * Initializes the front facing IR proximity sensor
 */
void Proxy_Init();

/*
 * Runs the IR read logic
 */
void IRRead();

/*
 * Returns max( left-LED-counts, right-LED-counts)
 */
uint16_t IR_Counts();

/*
 * This function starts the IR LED strobe based on the given
 * brightness level and set period. The logic for this function
 * is based off of wiring.c in the open-source Arduino-core library.
 */
void start_strobe(bool strobe_right, uint16_t brightness);

/*
 * This function stops the IR LED strobe. The logic for this function
 * is based off of wiring.c in the open-source Arduino-core library.
 */
void stop_strobe();

#endif
