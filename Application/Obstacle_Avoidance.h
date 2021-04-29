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
 * Obstacle_Avoidance.h/c
 *
 */
#ifndef OBSTACLE_AVOIDANCE_H
#define OBSTACLE_AVOIDANCE_H

#include <ctype.h>
#include <stdbool.h>

#include "../Driver/include_driver.h"
#include "application_defines.h"

#define OA_TURN_VELOCITY	1.0
#define OA_ANGLR_VELOCITY	1.0
#define OA_DRIVE_DC			25
#define OA_HOLD_TIMEOUT		200

typedef enum
{
	READ_L,
	READ_R,
	RUN_DSM	// Decision state machine
} eOAReadState;

// Obstacle Avoidance Decision State Machine
typedef enum
{
	INIT,
	DRIVE_STRAIGHT,
	TURN_LEFT,
	TURN_RIGHT,
	HOLDING
} eOADSM;

/*
 * Initializes the various state machines in obstacle avoidance logic.
 * This should be ran every time the user starts-up the obstacle
 * avoidance feature, even if it was called in the past.
 */
void Init_Obstacle_Avoidance();

// Run_OA_Task runs the over-arching obstacle avoidance logic
bool Run_OA_Task();

/*
 * Run_State_Machine runs the obstacle avoidance state machine logic
 * and handles sending drive commands to the driver layer. Takes the
 * current IR reading.
 */
void Run_State_Machine(t_ProximityReturn IR_Return);

#endif
