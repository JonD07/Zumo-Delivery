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

#include "Obstacle_Avoidance.h"

static eOAReadState read_state;
static eOADSM OA_state;
static uint16_t alpha;

/*
 * Initializes the various state machines in obstacle avoidance logic.
 * This should be ran every time the user starts-up the obstacle
 * avoidance feature, even if it was called in the past.
 */
void Init_Obstacle_Avoidance() {
	read_state = READ_L;
	OA_state = INIT;
	alpha = 0;
}

// Run_OA_Task runs the over-arching obstacle avoidance logic
bool Run_OA_Task() {
	// Don't do too much in one loop iteration
	bool run_again = false;
	// Run OA task state machine
	switch(read_state) {
	case READ_L:
		// Run read left command
		IRRead(LEFT);
		// Run task again and update state
		run_again = true;
		read_state = READ_R;
		break;

	case READ_R:
		// Run read right command
		IRRead(RIGHT);
		// Run task again and update state
		run_again = true;
		read_state = RUN_DSM;
		break;

	case RUN_DSM:
		// Run OA decision making state machine
		Run_State_Machine(IR_Counts());
		// Don't need to run again, let task sleep, reset state
		run_again = false;
		read_state = READ_L;
		break;

	default:
		// Something went wrong...
		run_again = false;
		read_state = READ_L;
	}

	return run_again;
}

/*
 * Run_State_Machine runs the obstacle avoidance state machine logic
 * and handles sending drive commands to the driver layer. Takes the
 * current IR reading.
 */
void Run_State_Machine(t_ProximityReturn ir_Return) {
	switch(OA_state) {
	case INIT:
	{
		float velocity_left = 0;
		float velocity_right = 0;
		// Initialize the movement based on IR reading
		if(ir_Return.m_nCount == 0) {
			// Drive straight
			velocity_left = DutyCycle_to_Velocity_Left(OA_DRIVE_DC);
			velocity_right = DutyCycle_to_Velocity_Right(OA_DRIVE_DC);
			// Update state
			OA_state = DRIVE_STRAIGHT;
		}
		else if(ir_Return.m_eSide == RIGHT) {
			// Turn left
			velocity_left = OA_TURN_VELOCITY;
			velocity_right = OA_TURN_VELOCITY + WHEEL_BASE * OA_ANGLR_VELOCITY;
			// Update state
			OA_state = TURN_LEFT;
		}
		else {
			// Turn right
			velocity_left = OA_TURN_VELOCITY + WHEEL_BASE * OA_ANGLR_VELOCITY;
			velocity_right = OA_TURN_VELOCITY;
			// Update state
			OA_state = TURN_RIGHT;
		}

		// Assign target velocity
		Controller_Set_Target_Velocity(&ctr_LeftMotor, velocity_left);
		Controller_Set_Target_Velocity(&ctr_RightMotor, velocity_right);

		// Set motor control flags
		mf_motor_vel_control.active = true;
		mf_motor_vel_control.duration = ctr_LeftMotor.update_period;

		// Turn on red LED
		Set_LED(RED, true);
		break;
	}

	case DRIVE_STRAIGHT:
		// Make movement decision
		if(ir_Return.m_nCount > 0) {
			// Make a turn
			float velocity_left = 0;
			float velocity_right = 0;

			if(ir_Return.m_eSide == RIGHT) {
				// Turn left
				velocity_left = OA_TURN_VELOCITY;
				velocity_right = OA_TURN_VELOCITY + WHEEL_BASE * OA_ANGLR_VELOCITY;
				// Update state
				OA_state = TURN_LEFT;
			}
			else {
				// Turn right
				velocity_left = OA_TURN_VELOCITY + WHEEL_BASE * OA_ANGLR_VELOCITY;
				velocity_right = OA_TURN_VELOCITY;
				// Update state
				OA_state = TURN_RIGHT;
			}

			// Assign target velocity
			Controller_Set_Target_Velocity(&ctr_LeftMotor, velocity_left);
			Controller_Set_Target_Velocity(&ctr_RightMotor, velocity_right);
		}
		else {
			// Stay in drive-straight state
			OA_state = DRIVE_STRAIGHT;
		}

		break;

	case TURN_RIGHT:
		// Stop turning?
		if(ir_Return.m_nCount == 0) {
			// Move to holding state
			OA_state = HOLDING;
		}
		else {
			// Stay in turn-right state
			OA_state = TURN_RIGHT;
		}

		break;

	case TURN_LEFT:
		// Stop turning?
		if(ir_Return.m_nCount == 0) {
			// Move to holding state
			OA_state = HOLDING;
		}
		else {
			// Stay in turn-right state
			OA_state = TURN_LEFT;
		}

		break;

	case HOLDING:
		alpha++;
		// Stop turning?
		if(alpha >= OA_HOLD_TIMEOUT) {
			// Reset alpha
			alpha = 0;
			// Drive straight, Assign target velocity
			Controller_Set_Target_Velocity(&ctr_LeftMotor, DutyCycle_to_Velocity_Left(OA_DRIVE_DC));
			Controller_Set_Target_Velocity(&ctr_RightMotor, DutyCycle_to_Velocity_Right(OA_DRIVE_DC));
			// Move to drive straight state
			OA_state = DRIVE_STRAIGHT;
		}
		else {
			// Stay in turn-right state
			OA_state = HOLDING;
		}

		break;
	}
}
