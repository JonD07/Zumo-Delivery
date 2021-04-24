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

#include "../Driver/SerialIO.h"
#include "../Driver/MEGN540_MessageHandeling.h"
#include "../Driver/Timing.h"
#include "../Driver/Encoder.h"
#include "../Driver/MotorPWM.h"
#include "../Driver/Filter.h"
#include "../Driver/Battery_Monitor.h"
#include "../Driver/Controller.h"
// #include "../c_lib/Task_Scheduler.h"

#define DEBUG		1
#define CNTRL_SYS	0
#define KP_L		0.1875335
#define KP_R		0.1728258

float num_left[] = {0.187533508705,		0.124897316798}; // b coefficients
float num_right[] = {0.172825818795,	0.130440286735};
float den_left[] = {1.000000000000,		-1.000000000000}; // a coefficients
float den_right[] = {1.000000000000,	-1.000000000000};

void InitializeSystem()
{

    
}

/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main(void)
{
	/// Basic setup tasks
	float bat_val = 0;

	//Motor Control Values
	float ticksL_old;
	float ticksR_old;
	float time_old;
	float ticksL_new;
	float ticksR_new;
	float time_new;
	bool first_time = true;

	// Initialize timer zero functionality
	SetupTimer0();
	// Initialize encoders
	Encoders_Init();
	// Initialize battery monitor
	Battery_Monitor_Init();
	// Initialize PMW
	Motor_PWM_Init(0x190); // 400 => 20 kHz
	// Initialize the left and right motor controller
	Controller_Init(&ctr_LeftMotor, KP_L, num_left, den_left, 1, 10); // 10ms => 100 Hz
	Controller_Init(&ctr_RightMotor, KP_R, num_right, den_right, 1, 10); // 10ms => 100 Hz
	// Initialize USB
	USB_SetupHardware();
	// Initialize message handling
	Message_Handling_Init();
	// Enable Global Interrupts for USB and Timer etc.
	GlobalInterruptEnable();

	// Init batter task flag
	mf_battery_task.duration = 2;
	mf_battery_task.active = true;

	while(true)
	{
		// USB serial comms up-keep
		USB_Upkeep_Task();
		// Manage USB messaging
		Message_Handling_Task();

		// Reset message handling
		if(MSG_FLAG_Execute(&mf_restart))
		{
			// Reinitialize stuff...
			Message_Handling_Init();
		}

		// Process battery low
		if(MSG_FLAG_Execute(&mf_battery_task))
		{
			// Update battery monitoring
			bat_val = Battery_Voltage_Task();
			if((bat_val < 4.75) && (bat_val > 3.0)) // battery low condition
			{
				mf_low_battery.active = true;
				mf_low_battery.duration = 1000;
			}
			else
			{
				mf_low_battery.active = false;
			}

			mf_battery_task.last_trigger_time = GetTime();
		}

		// Process send-time command
		if(MSG_FLAG_Execute(&mf_send_time))
		{
			// Get current time
			Time_t now = GetTime();
			float ret_val = ((float)now.millisec + now.microsec/1000.0)/1000.0;

			if(mf_send_time.duration < 0)
			{
				mf_send_time.active = false;
				// Send response
				Send_Time_Message('t', 0x00, ret_val);
			}
			else
			{
				// Send response
				Send_Time_Message('T', 0x00, ret_val);
				mf_send_time.last_trigger_time = GetTime();
			}
		}

		// Process send-float-time command
		if(MSG_FLAG_Execute(&mf_time_float_send))
		{
			static bool trip = false;
			static Time_t start_time;

			if(!trip)
			{
				float data = 12.345;
				start_time = GetTime();
				usb_send_msg("cf", 'f', &data, sizeof(data));
				trip = true;
			}
			else
			{
				if(usb_out_msg_length() == 0)
				{
					// Get current time
					Time_t ret_time = SecondsSince(&start_time);
					float ret_val = ((float)ret_time.millisec + ret_time.microsec/1000.0)/1000.0;

					if(mf_time_float_send.duration < 0)
					{
						mf_time_float_send.active = false;
						Send_Time_Message('t', 0x01, ret_val);
					}
					else
					{
						mf_time_float_send.last_trigger_time = GetTime();
						Send_Time_Message('T', 0x01, ret_val);
					}

					trip = false;
				}
			}
		}

		// Process loop-time command
		if(MSG_FLAG_Execute(&mf_loop_timer))
		{
			static bool trip = false;
			static Time_t start_time;

			if(!trip)
			{
				start_time = GetTime();
				trip = true;
			}
			else
			{
				// Get current time
				Time_t ret_time = SecondsSince(&start_time);
				float ret_val = ((float)ret_time.millisec + ret_time.microsec/1000.0)/1000.0;

				if(mf_loop_timer.duration < 0)
				{
					mf_loop_timer.active = false;
					Send_Time_Message('t', 0x02, ret_val);
				}
				else
				{
					mf_loop_timer.last_trigger_time = GetTime();
					Send_Time_Message('T', 0x02, ret_val);
				}

				trip = false;
			}
		}

		// Process encoder count command
		if(MSG_FLAG_Execute(&mf_send_encoder))
		{
			struct __attribute__((__packed__)) { float left_enc; float right_enc; } ret_val;
			ret_val.left_enc = Counts_Left();
			ret_val.right_enc = Counts_Right();

			if(mf_send_encoder.duration <= 0)
			{
				mf_send_encoder.active = false;
				usb_send_msg("cff", 'e', &ret_val, sizeof(ret_val));
			}
			else
			{
				mf_send_encoder.last_trigger_time = GetTime();
				usb_send_msg("cff", 'E', &ret_val, sizeof(ret_val));
			}
		}

		// Process battery monitor command
		if(MSG_FLAG_Execute(&mf_send_battery))
		{
			float ret_val = Battery_Voltage();

			if(mf_send_battery.duration <= 0)
			{
				mf_send_battery.active = false;
				usb_send_msg("cf", 'b', &ret_val, sizeof(ret_val));
			}
			else
			{
				mf_send_battery.last_trigger_time = GetTime();
				usb_send_msg("cf", 'B', &ret_val, sizeof(ret_val));
			}
		}

		// Process battery low
		if(MSG_FLAG_Execute(&mf_low_battery))
		{
			struct {char let[7]; float volts; } data =
			{
					.let = {'B', 'A', 'T', ' ', 'L', 'O', 'W'},
					.volts = bat_val
			};

			usb_send_msg("ccccccccf", '!', &data, sizeof(data));

			mf_low_battery.last_trigger_time = GetTime();
		}

		// Timed PWM flag
		if(MSG_FLAG_Execute(&mf_timed_pwm))
		{
			// Stop PWM
			Motor_PWM_Left(0);
			Motor_PWM_Right(0);

			// Disable PWM
			Motor_PWM_Enable(false);

			// Reset flag
			mf_timed_pwm.active = false;
		}

		// Handle system stats flag
		if(MSG_FLAG_Execute(&mf_sys_data))
		{
			// Create struct for data to return
			struct {float time; int16_t PWM_L; int16_t PWM_R; int16_t Encoder_L; int16_t Encoder_R; } data;
			// Get current time
			Time_t now = GetTime();
			data.time = ((float)now.millisec + now.microsec/1000.0)/1000.0;
			// Get PWM info
			data.PWM_L = Get_Motor_PWM_Left();
			data.PWM_R = Get_Motor_PWM_Right();
			// Get encoder readings
			data.Encoder_L = Counts_Left();
			data.Encoder_R = Counts_Right();

			if(mf_sys_data.duration <= 0)
			{
				mf_sys_data.active = false;
				usb_send_msg( "cfhhhh", 'q', &data, sizeof(data) );
			}
			else
			{
				mf_sys_data.last_trigger_time = GetTime();
				usb_send_msg( "cfhhhh", 'Q', &data, sizeof(data) );
			}
		}

		// Handle Motor Controls Flag
		if(MSG_FLAG_Execute(&mf_motor_dist_control))
		{
			if(first_time)
			{
				ticksL_old = Counts_Left();
				ticksR_old = Counts_Right();
				time_old = GetTimeSec();
				first_time = false;
			}
			else {
				ticksL_new = Counts_Left();
				ticksR_new = Counts_Right();
				float measured_left = ECount_to_Distance(ticksL_new - ticksL_old);
				float measured_right = ECount_to_Distance(ticksR_new - ticksR_old);

				// Determine if we are done moving
				bool doneL = (ctr_LeftMotor.target_pos > 0) ?
						(measured_left >= ctr_LeftMotor.target_pos)
						: (measured_left <= ctr_LeftMotor.target_pos);
				bool doneR = (ctr_RightMotor.target_pos > 0) ?
						(measured_right >= ctr_RightMotor.target_pos)
						: (measured_right <= ctr_RightMotor.target_pos);

				if(doneL || doneR)
				{
					Motor_PWM_Left(0);
					Motor_PWM_Right(0);

					// Disable PWM
					Motor_PWM_Enable(false);

					Controller_Set_Target_Position(&ctr_LeftMotor, 0.0);
					Controller_Set_Target_Position(&ctr_RightMotor, 0.0);
					Controller_Set_Target_Velocity(&ctr_LeftMotor, 0.0);
					Controller_Set_Target_Velocity(&ctr_RightMotor, 0.0);

					mf_motor_dist_control.active = false;
					first_time = true;
				}
				else
				{
					int16_t pwmL = 0;
					int16_t pwmR = 0;
					time_new = GetTimeSec();
					float dt = time_new - time_old;

					// Update target position
					Controller_Set_Target_Position(&ctr_LeftMotor,
							(ctr_LeftMotor.target_pos - measured_left));
					Controller_Set_Target_Position(&ctr_RightMotor,
							(ctr_RightMotor.target_pos - measured_right));

					if(CNTRL_SYS) {
						/// Update controller
						// Correct to keep measurement positive in controller
						float mL = (measured_left < 0) ? (-1*measured_left) : measured_left;
						float mR = (measured_right < 0) ? (-1*measured_right) : measured_right;
						float new_speedL = Controller_Update(&ctr_LeftMotor, mL, dt);
						float new_speedR = Controller_Update(&ctr_RightMotor, mR, dt);
						// Determine new PWM with sign for direction
						pwmL = Velocity_to_DutyCycle_Left(new_speedL);
						pwmR =  Velocity_to_DutyCycle_Right(new_speedR);
					}
					else {
						// Just use given velocity
						pwmL = Velocity_to_DutyCycle_Left(ctr_LeftMotor.target_vel);
						pwmR =  Velocity_to_DutyCycle_Right(ctr_RightMotor.target_vel);
					}

					pwmL = (ctr_LeftMotor.target_pos > 0) ? pwmL : -1 * pwmL;
					pwmR = (ctr_RightMotor.target_pos > 0) ? pwmR : -1 * pwmR;

					// Set PWM
					Motor_PWM_Left(pwmL);
					Motor_PWM_Right(pwmR);

					// Enable PWM
					Motor_PWM_Enable(true);

					if(DEBUG)
					{
						struct {float mL; float mR; int16_t valL; int16_t valR; } data =
						{
								.mL = measured_left,
								.mR = measured_right,
								.valL = pwmL,
								.valR = pwmR
						};

						usb_send_msg("cffhh", '!', &data, sizeof(data));
					}

					// Update for next loops
					ticksL_old = ticksL_new;
					ticksR_old = ticksR_new;
					time_old = time_new;
					mf_motor_dist_control.last_trigger_time = GetTime();
				}
			}
		}

		// Handle Motor Controls Flag
		if(MSG_FLAG_Execute(&mf_motor_vel_control))
		{
			if(first_time)
			{
				ticksL_old = Counts_Left();
				ticksR_old = Counts_Right();
				time_old = GetTimeSec();
				first_time = false;
			}
			else {
				// Update controller
				time_new = GetTimeSec();
				int16_t pwmL = 0;
				int16_t pwmR = 0;

				if(ctr_LeftMotor.target_vel < 0) {
					// Go backwards
					// Determine distance traveled
					ticksL_new = Counts_Left();
					ticksR_new = Counts_Right();

					if(CNTRL_SYS) {
						// Correct direction for controller
						ctr_RightMotor.target_vel = -1*ctr_RightMotor.target_vel;
						ctr_LeftMotor.target_vel = -1*ctr_LeftMotor.target_vel;
						float new_speedR = Controller_Update(&ctr_RightMotor, ((ticksR_old - ticksR_new) * 2 * 0.0195 / (12 * 75.81)), (time_new - time_old));
						float new_speedL = Controller_Update(&ctr_LeftMotor, ((ticksL_old - ticksL_new) * 2 * 0.0195 / (12 * 75.81)), (time_new - time_old));
						// Determine new PWM with sign for direction
						pwmL =  -1*(int16_t)((new_speedL - 0.0133) / 0.0033);
						pwmR =  -1*(int16_t)((new_speedR - 0.0133) / 0.0034);
						// Correct direction for controller
						ctr_RightMotor.target_vel = -1*ctr_RightMotor.target_vel;
						ctr_LeftMotor.target_vel = -1*ctr_LeftMotor.target_vel;
					}
					else {
						// Just use given velocity
						pwmL =  (int16_t)((ctr_LeftMotor.target_vel - 0.0133) / 0.0033);
						pwmR =  (int16_t)((ctr_RightMotor.target_vel - 0.0133) / 0.0034);
					}
				}
				else {
					// Go forwards
					// Determine distance traveled
					ticksL_new = Counts_Left();
					ticksR_new = Counts_Right();

					if(CNTRL_SYS) {
						float new_speedR = Controller_Update(&ctr_RightMotor, ((ticksR_new - ticksR_old) * 2 * 0.0195 / (12 * 75.81)), (time_new - time_old));
						float new_speedL = Controller_Update(&ctr_LeftMotor, ((ticksL_new - ticksL_old) * 2 * 0.0195 / (12 * 75.81)), (time_new - time_old));
						// Determine new PWM with sign for direction
						pwmL =  (int16_t)((new_speedL - 0.0133) / 0.0033);
						pwmR =  (int16_t)((new_speedR - 0.0133) / 0.0034);
					}
					else {
						// Just use given velocity
						pwmL =  (int16_t)((ctr_LeftMotor.target_vel - 0.0133) / 0.0033);
						pwmR =  (int16_t)((ctr_RightMotor.target_vel - 0.0133) / 0.0034);
					}
				}

				// Set PWM
				Motor_PWM_Left(pwmL);
				Motor_PWM_Right(pwmR);

				// Enable PWM
				Motor_PWM_Enable(true);

				if(DEBUG)
				{
					struct { int16_t valL; int16_t valR; } data =
					{
							.valL = pwmL,
							.valR = pwmR
					};

					usb_send_msg("chh", '!', &data, sizeof(data));
				}

				// Update for next loops
				ticksL_old = ticksL_new;
				ticksR_old = ticksR_new;
				time_old = time_new;
				mf_motor_vel_control.last_trigger_time = GetTime();
			}
		}

		// Handle Motor Stop Flag
		if(MSG_FLAG_Execute(&mf_motor_stop))
		{
			// Shut-off PWM
			Motor_PWM_Left(0);
			Motor_PWM_Right(0);

			// Disable PWM
			Motor_PWM_Enable(false);

			// Reset controllers
			Controller_Set_Target_Position(&ctr_LeftMotor, 0.0);
			Controller_Set_Target_Position(&ctr_RightMotor, 0.0);
			Controller_Set_Target_Velocity(&ctr_LeftMotor, 0.0);
			Controller_Set_Target_Velocity(&ctr_RightMotor, 0.0);

			// Reset flag
			mf_motor_stop.active = false;
			mf_motor_dist_control.active = false;
			mf_motor_vel_control.active = false;
			mf_timed_pwm.active = false;
		}
	}
}



