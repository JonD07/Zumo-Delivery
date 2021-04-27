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

#include "MEGN540_MessageHandeling.h"


static inline void MSG_FLAG_Init(MSG_FLAG_t* p_flag)
{
    p_flag->active = false;
    p_flag->duration = -1;
    p_flag->last_trigger_time.millisec=0;
    p_flag->last_trigger_time.microsec=0;
}


/**
 * Function MSG_FLAG_Execute indicates if the action associated with the message flag should be executed
 * in the main loop both because its active and because its time.
 * @return [bool] True for execute action, False for skip action
 */
bool MSG_FLAG_Execute( MSG_FLAG_t* p_flag)
{
	if(p_flag->active)
	{
		// Determine time since flag was last set
		Time_t delta_time = SecondsSince(&p_flag->last_trigger_time);

		if((float)delta_time.millisec >= p_flag->duration)
		{
			return true;
		}
	}

	return false;
}


/**
 * Function Message_Handling_Init initializes the message handling and all associated state flags and data to their default
 * conditions.
 */
void Message_Handling_Init()
{
	// Init restart flag to default values
	MSG_FLAG_Init( &mf_restart );
	MSG_FLAG_Init( &mf_loop_timer );
	MSG_FLAG_Init( &mf_time_float_send );
	MSG_FLAG_Init( &mf_send_time );
	MSG_FLAG_Init( &mf_send_encoder );
	MSG_FLAG_Init( &mf_send_battery );
	MSG_FLAG_Init( &mf_low_battery );
	MSG_FLAG_Init( &mf_timed_pwm );
	MSG_FLAG_Init( &mf_sys_data );
	MSG_FLAG_Init( &mf_motor_dist_control );
	MSG_FLAG_Init( &mf_motor_stop );
	MSG_FLAG_Init( &mf_ir_proximity );
}

/**
 * Function Message_Handler processes USB messages as necessary and sets status flags to control the flow of the program.
 * It returns true unless the program receives a reset message.
 * @return
 */
void Message_Handling_Task()
{
	// Check to see if their is data in waiting
	if( !usb_msg_length() )
		return; // nothing to process...

	// Get Your command designator without removal so if their are not enough bytes yet, the command persists
	char command = usb_msg_peek();

	// process command
	switch( command )
	{
	case '*':
		if( usb_msg_length() >= MEGN540_Message_Len('*') )
		{
			// remove the command from the usb received buffer
			usb_msg_get();

			// Build a meaningful structure to put your data in. Here we want two floats.
			struct __attribute__((__packed__)) { float v1; float v2; } data;

			// Copy the bytes from the usb receive buffer into our structure so we can use the information
			usb_msg_read_into( &data, sizeof(data) );

			// Do the thing you need to do. Here we want to multiply
			float ret_val = data.v1 * data.v2;

			// Send response
			usb_send_msg("cf", command, &ret_val, sizeof(ret_val));
		}
		break;

	case '/':
		if( usb_msg_length() >= MEGN540_Message_Len('/') )
		{
			// Remove first byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { float v1; float v2; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );

			// NOTE: This might not work. fpclassify() from math.h isn't compiling (need -lm flag)
			//		 and I haven't taken the time to write my own function.
			if(data.v2 != 0.0f)
			{
				// Do math
				float ret_val = data.v1 / data.v2;
				// Return answer
				usb_send_msg("cf", command, &ret_val, sizeof(ret_val));
			}
			else // Divide by zero...
			{
				// Send '?' back to user
				char bad_input = '?';
				usb_send_msg("cc", command, &bad_input, sizeof(bad_input));
			}
		}
		break;

	case '+':
		if( usb_msg_length() >= MEGN540_Message_Len('+') )
		{
			// Remove first byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { float v1; float v2; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );
			// Do math
			float ret_val = data.v1 + data.v2;
			// Return answer
			usb_send_msg("cf", command, &ret_val, sizeof(ret_val));
		}
		break;

	case '-':
		if( usb_msg_length() >= MEGN540_Message_Len('-') )
		{
			// Remove first byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { float v1; float v2; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );
			// Do math
			float ret_val = data.v1 - data.v2;
			// Return answer
			usb_send_msg("cf", command, &ret_val, sizeof(ret_val));
		}
		break;

	case '~':
		if( usb_msg_length() >= MEGN540_Message_Len('~') )
		{
			mf_restart.active = true;
		}
		break;

	case 't':
		if( usb_msg_length() >= MEGN540_Message_Len('t') )
		{
			// Remove first byte
			usb_msg_get();

			// Get the action command specified by user, setting the corresponding flag to active.
			char action = usb_msg_get();

			if (action == 0x00){
				mf_send_time.active = true;
				mf_send_time.duration = -1;
			} else if (action == 0x01){
				mf_time_float_send.active = true;
				mf_time_float_send.duration = -1;
			} else if (action == 0x02){
				mf_loop_timer.active = true;
				mf_loop_timer.duration = -1;
			} else {
				char bad_input = '?';
				usb_send_msg("cc", command, &bad_input, sizeof(bad_input));
			}
		}
		break;

	case 'T':
		if( usb_msg_length() >= MEGN540_Message_Len('T') )
		{
			// Remove first byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { char action; float duration; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );

			if (data.action == 0x00 && data.duration > 0.0){
				mf_send_time.active = true;
				// Convert duration from s to ms
				mf_send_time.duration = data.duration * 1000;
			} else if (data.action == 0x01 && data.duration > 0.0){
				mf_time_float_send.active = true;
				// Convert duration from s to ms
				mf_time_float_send.duration = data.duration * 1000;
			} else if (data.action == 0x02 && data.duration > 0.0){
				mf_loop_timer.active = true;
				// Convert duration from s to ms
				mf_loop_timer.duration = data.duration * 1000;
			} else {
				char bad_input = '?';
				usb_send_msg("cc", command, &bad_input, sizeof(bad_input));
			}
		}
		break;

	case 'e':
		if( usb_msg_length() >= MEGN540_Message_Len('e') )
		{
			// Remove first byte
			usb_msg_get();

			mf_send_encoder.active = true;
			mf_send_encoder.duration = -1;
		}
		break;

	case 'E':
		if( usb_msg_length() >= MEGN540_Message_Len('E') )
		{
			// Remove first byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { float duration; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );

			if (data.duration > 0.0)
			{
				mf_send_encoder.active = true;
				// Convert duration from s to ms
				mf_send_encoder.duration = data.duration * 1000;
			} else {
				char bad_input = '?';
				usb_send_msg("cc", command, &bad_input, sizeof(bad_input));
			}
		}
		break;

	case 'b':
		if( usb_msg_length() >= MEGN540_Message_Len('b') )
		{
			// Remove first byte
			usb_msg_get();

			mf_send_battery.active = true;
			mf_send_battery.duration = -1;
		}
		break;

	case 'B':
		if( usb_msg_length() >= MEGN540_Message_Len('B') )
		{
			// Remove first byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { float duration; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );

			if (data.duration > 0.0)
			{
				mf_send_battery.active = true;
				// Convert duration from s to ms
				mf_send_battery.duration = data.duration * 1000;
			} else {
				char bad_input = '?';
				usb_send_msg("cc", command, &bad_input, sizeof(bad_input));
			}
		}
		break;

	case 'p':	// Set PWM signal
		if( usb_msg_length() >= MEGN540_Message_Len('p') )
		{
			// Remove char byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { int left; int right; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );

			float bat_val = Battery_Voltage();

			if(bat_val >= 4.75)
			{
				// Set left and right duty cycle
				Motor_PWM_Left(data.left);
				Motor_PWM_Right(data.right);

				// Enable PWM
				Motor_PWM_Enable(true);

				// Deactivate other flags
				mf_motor_dist_control.active = false;
				mf_motor_vel_control.active = false;
				mf_motor_stop.active = false;
				mf_timed_pwm.active = false;
			} else if(bat_val > 3.5) {	// Battery low!
				// Create struct for message
				struct {char let[7]; float volts; } data =
				{
						.let = {'B', 'A', 'T', ' ', 'L', 'O', 'W'},
						.volts = bat_val
				};
				// Send bat-low message
				usb_send_msg("ccccccccf", '!', &data, sizeof(data));
			} else { // Battery probably turned off
				// Create struct for message
				struct {char let[9]; float volts; } data =
				{
						.let = {'P', 'O', 'W', 'E', 'R', ' ', 'O', 'F', 'F'},
						.volts = bat_val
				};
				// Send bat-off message
				usb_send_msg("ccccccccccf", '!', &data, sizeof(data));
			}
		}
		break;

	case 'P':	// Set PWM signal for duration
		if( usb_msg_length() >= MEGN540_Message_Len('P') )
		{
			// Remove char byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { int left; int right; float duration; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );

			float bat_val = Battery_Voltage();

			if(bat_val >= 4.75)
			{
				if (data.duration > 0.0)
				{
					// Set left and right duty cycle
					Motor_PWM_Left(data.left);
					Motor_PWM_Right(data.right);

					// Enable PWM
					Motor_PWM_Enable(true);

					mf_timed_pwm.active = true;
					mf_timed_pwm.last_trigger_time = GetTime();
					// Convert duration from s to ms
					mf_timed_pwm.duration = data.duration * 1000;

					// Deactivate other flags
					mf_motor_dist_control.active = false;
					mf_motor_vel_control.active = false;
					mf_motor_stop.active = false;
				} else {
					char bad_input = '?';
					usb_send_msg("cc", command, &bad_input, sizeof(bad_input));
				}
			} else if(bat_val > 3.5) {	// Battery low!
				// Create struct for message
				struct {char let[7]; float volts; } data =
				{
						.let = {'B', 'A', 'T', ' ', 'L', 'O', 'W'},
						.volts = bat_val
				};
				// Send bat-low message
				usb_send_msg("ccccccccf", '!', &data, sizeof(data));
			} else { // Battery probably turned off
				// Create struct for message
				struct {char let[9]; float volts; } data =
				{
						.let = {'P', 'O', 'W', 'E', 'R', ' ', 'O', 'F', 'F'},
						.volts = bat_val
				};
				// Send bat-off message
				usb_send_msg("ccccccccccf", '!', &data, sizeof(data));
			}
		}
		break;

	case 's':	// Stop motors
	case 'S':
		if( usb_msg_length() >= MEGN540_Message_Len('s') )
		{
			// Remove char byte
			usb_msg_get();

			// Disable PWM
			Motor_PWM_Enable(false);
			// Stop PWM
			Motor_PWM_Left(0);
			Motor_PWM_Right(0);

			// Stop controllers
			Controller_Set_Target_Position(&ctr_LeftMotor, 0.0);
			Controller_Set_Target_Position(&ctr_RightMotor, 0.0);
			Controller_Set_Target_Velocity(&ctr_LeftMotor, 0.0);
			Controller_Set_Target_Velocity(&ctr_RightMotor, 0.0);

			// Deactivate other flags
			mf_motor_dist_control.active = false;
			mf_motor_vel_control.active = false;
			mf_motor_stop.active = false;
			mf_timed_pwm.active = false;
		}
		break;

	case 'q':	// Send system status
		if( usb_msg_length() >= MEGN540_Message_Len('q') )
		{
			// Remove first byte
			usb_msg_get();

			mf_sys_data.active = true;
			mf_sys_data.duration = -1;
		}
		break;

	case 'Q':	// Send system status
		if( usb_msg_length() >= MEGN540_Message_Len('Q') )
		{
			// Remove first byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { float duration; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );

			if(data.duration > 0.0)
			{
				mf_sys_data.active = true;
				mf_sys_data.duration = data.duration * 1000;
			} else {
				char bad_input = '?';
				usb_send_msg("cc", command, &bad_input, sizeof(bad_input));
			}
		}
		break;

	/*
	 * How does turning work...?
	 *
	 * When you send an angle (in radians) to the car with the 'd'
	 * or 'D' commands the car will turn that many radians around a
	 * circle. The linear distance component is used as the arc length
	 * of the inner track. The velocity is set to a defined constant
	 * for the inner track and the outer track's velocity is based on
	 * the distance that has to be covered to turn the given radians.
	 *
	 * The sign of the given angle determines the direction of the
	 * turn. A positive angle will turn the car left and a negative
	 * angle will turn the car right. Passing a negative distance to
	 * the car while making a turn will not change anything, this
	 * distance is corrected to be positive. The car does not turn
	 * backwards
	 *
	 * If the given distance is too small, the will simple spin like
	 * a top.
	 */
	case 'd':
		if( usb_msg_length() >= MEGN540_Message_Len('d') )
		{
			// Remove char byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { float linear; float angular; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );

			float bat_val = Battery_Voltage();

			// Check power levels
			if(bat_val >= 4.75)
			{
				float distance_left = 0;
				float distance_right = 0;
				float velocity_left = 0;
				float velocity_right = 0;

				if((data.angular < 0.01) && (data.angular > -0.01)) {
					if(1) {
						char out_put[] = {'G','o',' ','S','t','r'};
						usb_send_msg("ccccccc", command, &out_put, sizeof(out_put));
					}
					// Drive straight
					// Assign linear position
					distance_left = data.linear;
					distance_right = data.linear;
					// Set the desired velocity to 75% PWM
					velocity_left = DutyCycle_to_Velocity_Left(75);
					velocity_right = DutyCycle_to_Velocity_Right(75);
				}
				else {
					if(1) {
						char out_put[] = {'T','u','r','n'};
						usb_send_msg("ccccc", command, &out_put, sizeof(out_put));
					}
					if(data.linear < 0) {
						// Don't accept negative distances in turns, just set to positive
						data.linear *= -1;
					}

					/// Move car around circle
					if(data.linear < MIN_TURN_ARC) {
						// Just spin
						float d = (HALF_WHEEL_BASE * data.angular)/2;
						if(data.angular > 0) {
							// Spin left
							distance_left = -1 * d;
							distance_right = d;
						}
						else {
							// Spin right
							distance_left = d;
							distance_right = -1 * d;
						}

						velocity_left = DutyCycle_to_Velocity_Left(SPIN_DUTYCYLE);
						velocity_right = DutyCycle_to_Velocity_Right(SPIN_DUTYCYLE);
					}
					else {
						// Determine distance and velocity to travel on left and right
						if(data.angular > 0) { // Turn left
							// Arc length of inner track based on given distance
							distance_left = data.linear;
							// Arc length of outer track based on radius of turn & given angle
							float r = (data.linear/data.angular);
							distance_right = data.angular * (r + WHEEL_BASE);

							// Set velocities
							velocity_left = TURN_VELOCITY;
							float dt = distance_left/TURN_VELOCITY;
							velocity_right = distance_right/dt;
						}
						else { // Turn right
							// Correct angle for math
							data.angular *= -1;
							// Arc length of inner track based on given distance
							distance_right = data.linear;
							// Arc length of outer track based on radius of turn & given angle
							float r = (data.linear/data.angular);
							distance_left = data.angular * (r + WHEEL_BASE);

							// Set velocities
							velocity_right = TURN_VELOCITY;
							float dt = distance_right/TURN_VELOCITY;
							velocity_left = distance_left/dt;
						}
					}
				}

				// Assign linear position
				Controller_Set_Target_Position(&ctr_LeftMotor, distance_left);
				Controller_Set_Target_Position(&ctr_RightMotor, distance_right);
				// Assign target velocity
				Controller_Set_Target_Velocity(&ctr_LeftMotor, velocity_left);
				Controller_Set_Target_Velocity(&ctr_RightMotor, velocity_right);
				Zero_Encoders();

				// Set flags
				mf_motor_dist_control.active = true;
				mf_motor_dist_control.duration = ctr_LeftMotor.update_period;

				// Deactivate other flags
				mf_motor_vel_control.active = false;
				mf_motor_stop.active = false;
				mf_timed_pwm.active = false;
			}
			else if(bat_val < 3.0) // Battery Low
			{
				// Create struct for message
				struct {char let[7]; float volts; } data =
				{
						.let = {'B', 'A', 'T', ' ', 'L', 'O', 'W'},
						.volts = bat_val
				};
				// Send bat-low message
				usb_send_msg("ccccccccf", '!', &data, sizeof(data));
			}
			else // Power Off
			{
				// Create struct for message
				struct {char let[9]; float volts; } data =
				{
						.let = {'P', 'O', 'W', 'E', 'R', ' ', 'O', 'F', 'F'},
						.volts = bat_val
				};
				// Send bat-off message
				usb_send_msg("ccccccccccf", '!', &data, sizeof(data));
			}
		}
		break;


	// Specifies distance to drive, linear followed by angular
	// Terminates after X milliseconds (if negative car stops)
	/*
	 * How does turning work...?
	 *
	 * A detailed description is given for the 'd' command. Please
	 * review the comments there to understand how to properly make
	 * the car turn.
	 */
	case 'D':
		if( usb_msg_length() >= MEGN540_Message_Len('D') )
		{
			// Remove char byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { float linear; float angular; float duration; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );

			float bat_val = Battery_Voltage();

			// Check power levels
			if(bat_val >= 4.75)
			{
				float distance_left = 0;
				float distance_right = 0;
				float velocity_left = 0;
				float velocity_right = 0;

				if((data.angular < 0.01) && (data.angular > -0.01)) {
					if(1) {
						char out_put[] = {'G','o',' ','S','t','r'};
						usb_send_msg("ccccccc", command, &out_put, sizeof(out_put));
					}
					// Drive straight
					// Assign linear position
					distance_left = data.linear;
					distance_right = data.linear;
					// Set the desired velocity to 75% PWM
					velocity_left = DutyCycle_to_Velocity_Left(75);
					velocity_right = DutyCycle_to_Velocity_Right(75);
				}
				else {
					if(1) {
						char out_put[] = {'T','u','r','n'};
						usb_send_msg("ccccc", command, &out_put, sizeof(out_put));
					}
					if(data.linear < 0) {
						// Don't accept negative distances in turns, just set to positive
						data.linear *= -1;
					}

					/// Move car around circle
					if(data.linear < MIN_TURN_ARC) {
						// Just spin
						float d = (HALF_WHEEL_BASE * data.angular)/2;
						if(data.angular > 0) {
							// Spin left
							distance_left = -1 * d;
							distance_right = d;
						}
						else {
							// Spin right
							distance_left = d;
							distance_right = -1 * d;
						}

						velocity_left = DutyCycle_to_Velocity_Left(SPIN_DUTYCYLE);
						velocity_right = DutyCycle_to_Velocity_Right(SPIN_DUTYCYLE);
					}
					else {
						// Determine distance and velocity to travel on left and right
						if(data.angular > 0) { // Turn left
							// Arc length of inner track based on given distance
							distance_left = data.linear;
							// Arc length of outer track based on radius of turn & given angle
							float r = (data.linear/data.angular);
							distance_right = data.angular * (r + WHEEL_BASE);

							// Set velocities
							velocity_left = TURN_VELOCITY;
							float dt = distance_left/TURN_VELOCITY;
							velocity_right = distance_right/dt;
						}
						else { // Turn right
							// Correct angle for math
							data.angular *= -1;
							// Arc length of inner track based on given distance
							distance_right = data.linear;
							// Arc length of outer track based on radius of turn & given angle
							float r = (data.linear/data.angular);
							distance_left = data.angular * (r + WHEEL_BASE);

							// Set velocities
							velocity_right = TURN_VELOCITY;
							float dt = distance_right/TURN_VELOCITY;
							velocity_left = distance_left/dt;
						}
					}
				}

				// Assign linear position
				Controller_Set_Target_Position(&ctr_LeftMotor, distance_left);
				Controller_Set_Target_Position(&ctr_RightMotor, distance_right);
				// Assign target velocity
				Controller_Set_Target_Velocity(&ctr_LeftMotor, velocity_left);
				Controller_Set_Target_Velocity(&ctr_RightMotor, velocity_right);
				Zero_Encoders();

				// Set flags
				mf_motor_dist_control.active = true;
				mf_motor_dist_control.duration = ctr_LeftMotor.update_period;
				mf_motor_stop.active = true;
				mf_motor_stop.duration = data.duration * 1000;
				mf_motor_stop.last_trigger_time = GetTime();

				// Deactivate other flags
				mf_motor_vel_control.active = false;
				mf_timed_pwm.active = false;
			}
			else if(bat_val < 3.0) // Battery Low
			{
				// Create struct for message
				struct {char let[7]; float volts; } data =
				{
						.let = {'B', 'A', 'T', ' ', 'L', 'O', 'W'},
						.volts = bat_val
				};
				// Send bat-low message
				usb_send_msg("ccccccccf", '!', &data, sizeof(data));
			}
			else // Power Off
			{
				// Create struct for message
				struct {char let[9]; float volts; } data =
				{
						.let = {'P', 'O', 'W', 'E', 'R', ' ', 'O', 'F', 'F'},
						.volts = bat_val
				};
				// Send bat-off message
				usb_send_msg("ccccccccccf", '!', &data, sizeof(data));
			}
		}
		break;

	// Specifies speed to drive, linear followed by angular
	/*
	 * How does turning work...?
	 *
	 * For 'v' and 'V' commands, the car will turn when given a non-zero
	 * angular velocity (the second float). The given velocity is used
	 * for the inner track and the velocity for the outer track is based on
	 * the given angular velocity. To make the car turn left, pass in a
	 * positive angular velocity, and to turn right use a negative angular
	 * velocity.
	 */
	case 'v':
		if( usb_msg_length() >= MEGN540_Message_Len('v') )
		{
			// Remove char byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { float velocity; float angular; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );

			float bat_val = Battery_Voltage();

			// Check power levels
			if(bat_val >= 4.75)
			{
				float distance_left = 0;
				float distance_right = 0;
				float velocity_left = 0;
				float velocity_right = 0;

				if((data.angular < 0.01) && (data.angular > -0.01)) {
					// Drive straight
					if(1) {
						char out_put[] = {'G','o',' ','S','t','r'};
						usb_send_msg("ccccccc", command, &out_put, sizeof(out_put));
					}
					// Set the desired velocity to 75% PWM
					velocity_left = data.velocity;
					velocity_right = data.velocity;
				}
				else {
					if(1) {
						char out_put[] = {'T','u','r','n'};
						usb_send_msg("ccccc", command, &out_put, sizeof(out_put));
					}

					// Determine distance and velocity to travel on left and right
					if(data.angular > 0) { // Turn left
						// Set velocities
						velocity_left = data.velocity;
						velocity_right = data.velocity + WHEEL_BASE * data.angular;
					}
					else { // Turn right
						// Set velocities
						velocity_left = data.velocity + WHEEL_BASE * data.angular;
						velocity_right = data.velocity;
					}
				}

				// Assign linear position
				Controller_Set_Target_Position(&ctr_LeftMotor, distance_left);
				Controller_Set_Target_Position(&ctr_RightMotor, distance_right);
				// Assign target velocity
				Controller_Set_Target_Velocity(&ctr_LeftMotor, velocity_left);
				Controller_Set_Target_Velocity(&ctr_RightMotor, velocity_right);
				Zero_Encoders();

				// Set flags
				mf_motor_vel_control.active = true;
				mf_motor_vel_control.duration = ctr_LeftMotor.update_period;

				// Deactivate other flags
				mf_motor_dist_control.active = false;
				mf_motor_stop.active = false;
				mf_timed_pwm.active = false;
			}
			else if(bat_val < 3.0) // Battery Low
			{
				// Create struct for message
				struct {char let[7]; float volts; } data =
				{
						.let = {'B', 'A', 'T', ' ', 'L', 'O', 'W'},
						.volts = bat_val
				};
				// Send bat-low message
				usb_send_msg("ccccccccf", '!', &data, sizeof(data));
			}
			else // Power Off
			{
				// Create struct for message
				struct {char let[9]; float volts; } data =
				{
						.let = {'P', 'O', 'W', 'E', 'R', ' ', 'O', 'F', 'F'},
						.volts = bat_val
				};
				// Send bat-off message
				usb_send_msg("ccccccccccf", '!', &data, sizeof(data));
			}
		}
		break;

	// Specifies speed to drive, linear followed by angular
	// Terminates after X milliseconds (if negative car stops)
	/*
	 * How does turning work...?
	 *
	 * Please see comments above for 'v' command.
	 */
	case 'V':
		if( usb_msg_length() >= MEGN540_Message_Len('V') )
		{
			// Remove char byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { float velocity; float angular; float duration; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );

			float bat_val = Battery_Voltage();

			// Check power levels
			if(bat_val >= 4.75)
			{
				float distance_left = 0;
				float distance_right = 0;
				float velocity_left = 0;
				float velocity_right = 0;

				if((data.angular < 0.01) && (data.angular > -0.01)) {
					// Drive straight
					if(1) {
						char out_put[] = {'G','o',' ','S','t','r'};
						usb_send_msg("ccccccc", command, &out_put, sizeof(out_put));
					}
					// Set the desired velocity to 75% PWM
					velocity_left = data.velocity;
					velocity_right = data.velocity;
				}
				else {
					if(1) {
						char out_put[] = {'T','u','r','n'};
						usb_send_msg("ccccc", command, &out_put, sizeof(out_put));
					}

					// Determine distance and velocity to travel on left and right
					if(data.angular > 0) { // Turn left
						// Set velocities
						velocity_left = data.velocity;
						velocity_right = data.velocity + WHEEL_BASE * data.angular;
					}
					else { // Turn right
						// Set velocities
						velocity_left = data.velocity + WHEEL_BASE * data.angular;
						velocity_right = data.velocity;
					}
				}

				// Assign linear position
				Controller_Set_Target_Position(&ctr_LeftMotor, distance_left);
				Controller_Set_Target_Position(&ctr_RightMotor, distance_right);
				// Assign target velocity
				Controller_Set_Target_Velocity(&ctr_LeftMotor, velocity_left);
				Controller_Set_Target_Velocity(&ctr_RightMotor, velocity_right);
				Zero_Encoders();

				// Set flags
				mf_motor_vel_control.active = true;
				mf_motor_vel_control.duration = ctr_LeftMotor.update_period;
				mf_motor_stop.active = true;
				mf_motor_stop.duration = data.duration * 1000;
				mf_motor_stop.last_trigger_time = GetTime();

				// Deactivate other flags
				mf_motor_dist_control.active = false;
				mf_timed_pwm.active = false;
			}
			else if(bat_val < 3.0) // Battery Low
			{
				// Create struct for message
				struct {char let[7]; float volts; } data =
				{
						.let = {'B', 'A', 'T', ' ', 'L', 'O', 'W'},
						.volts = bat_val
				};
				// Send bat-low message
				usb_send_msg("ccccccccf", '!', &data, sizeof(data));
			}
			else // Power Off
			{
				// Create struct for message
				struct {char let[9]; float volts; } data =
				{
						.let = {'P', 'O', 'W', 'E', 'R', ' ', 'O', 'F', 'F'},
						.volts = bat_val
				};
				// Send bat-off message
				usb_send_msg("ccccccccccf", '!', &data, sizeof(data));
			}

		}
		break;

	case 'i':	// Send IR distances
		if( usb_msg_length() >= MEGN540_Message_Len('i') )
		{
			// Remove first byte
			usb_msg_get();

			mf_ir_proximity.active = true;
			mf_ir_proximity.duration = -1;
		}
		break;

	case 'I':	// Send IR distances
		if( usb_msg_length() >= MEGN540_Message_Len('I') )
		{
			// Remove first byte
			usb_msg_get();

			// Build structure to put data in
			struct __attribute__((__packed__)) { float duration; } data;

			// Copy the bytes from the usb receive buffer into structure
			usb_msg_read_into( &data, sizeof(data) );

			if(data.duration > 0.0)
			{
				mf_ir_proximity.active = true;
				mf_ir_proximity.duration = data.duration * 1000;
			} else {
				char bad_input = '?';
				usb_send_msg("cc", command, &bad_input, sizeof(bad_input));
			}
		}
		break;

	default:
		// Clear input buffer
		usb_flush_input_buffer();

		// Send '?' back to user
		char bad_input = '?';
		usb_send_msg("cc", command, &bad_input, sizeof(bad_input));

		break;
	}
}



/**
 * Function MEGN540_Message_Len returns the number of bytes associated with a command string per the
 * class documentation;
 * @param cmd
 * @return Size of expected string. Returns 0 if unreconized.
 */
uint8_t MEGN540_Message_Len( char cmd )
{
	switch(cmd)
	{
		case '~': return	1; break;
		case '*': return	9; break;
		case '/': return	9; break;
		case '+': return	9; break;
		case '-': return	9; break;
		case 't': return	2; break;
		case 'T': return	6; break;
		case 'e': return	1; break;
		case 'E': return	5; break;
		case 'b': return	1; break;
		case 'B': return	5; break;
		//        case 'a': return	1; break;
		//        case 'A': return 	5; break;
		//        case 'w': return	1; break;
		//        case 'W': return 	5; break;
		//        case 'm': return	1; break;
		//        case 'M': return	5; break;
		case 'p': return	5; break;
		case 'P': return	9; break;
		case 's': return 	1; break;
		case 'S': return 	1; break;
		case 'q': return	1; break;
		case 'Q': return 	5; break;
		case 'd': return 	9; break;
		case 'D': return	13; break;
		case 'v': return	9; break;
		case 'V': return	13; break;
		case 'i': return	1; break;
		case 'I': return	5; break;
		default:  return	0; break;
	}
}

/**
 *
 */
void Send_Time_Message(char cmd, uint8_t arg, float time)
{
	// Struct for return data
	struct __attribute__((__packed__)) { uint8_t arg; float duration; } ret_val;
	ret_val.arg = arg;
	ret_val.duration = time;

	usb_send_msg("ccf", cmd, &ret_val, sizeof(ret_val));
}
