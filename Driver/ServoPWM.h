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

/**
 * ServoPWM.h/c defines functions used to control the gripper servo motor. The
 * servo motor needs to run on an electrical pulse with a period of 20 ms (50 Hz)
 * and pulses between 1 ms and 2 ms. The motor will be turned completely in one
 * direction when the pulses are 1 ms long and 180 degrees in the opposite
 * direction when the pulses are 2 ms.
 *
 */
#ifndef SERVO_PWM_H
#define SERVO_PWM_H

#include <avr/interrupt.h> // for interrupt enable/disable
#include <avr/io.h>        // For pin input/output access
#include <ctype.h>         // For int32_t type
#include <stdbool.h>       // For bool

#include "driver_defines.h"

#define DUTY_CYCLE_MIN	8
#define DUTY_CYCLE_MAX	16
#define MAX_PWM			156

/**
 * Function ServoPWM_Init initializes the Servo PWM on Timer 1 for PWM based voltage control of the Servos.
 * The Servo PWM system shall initialize in the disabled state for safety reasons. You should specifically enable
 * Servo PWM outputs only as necessary.
 * @param [uint16_t] MAX_PWM is the maximum PWM value to use. This controls the PWM frequency.
 */
void Servo_PWM_Init();

/**
 * Function Close_Servo sets the PWM duty cycle for the servo to close.
 */
void Close_Servo();

/**
 * Function Open_Servo sets the PWM duty cycle for the servo to open.
 */
void Open_Servo();

/**
 * Function Servo_Is_Closed returns true if the gripper is closed
 */
bool Servo_Is_Closed();

#endif
