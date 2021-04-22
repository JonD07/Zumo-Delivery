#include "Controller.h"

//Function Initialize_Controller sets up the z-transform-based controller for the system
// update_period is in miliseconds
void Controller_Init(Controller_t* p_cont, float kp, float* num, float* den, uint8_t order, float update_period)
{
	Filter_Init(&p_cont->controller, num, den, order);
	p_cont->kp = kp;
	p_cont->update_period = update_period;
	p_cont->target_pos = 0;
	p_cont->target_vel = 0;

	return;
}

void Controller_Set_Target_Velocity(Controller_t* p_cont, float vel)
//Function Controller_Set_Target_Velocity sets the target velocity for the controller
{
	p_cont->target_vel = vel;
	return;
}

void Controller_Set_Target_Position(Controller_t* p_cont, float pos)
/*Function Controller_Set_Target_Position sets the target position for the controller
This also sets the target velocity to zero */
{
//	p_cont->target_vel = 0;
	p_cont->target_pos = pos;
	return;
}

/*
 * desired -->(+  )-->[PI]--->[G]----> output
 *              -                  |
 *              |------------------|
 */
float Controller_Update(Controller_t* p_cont, float measurement, float dt)
//Function Controller_Update takes in a new measurement and returns the new control value
{
	// Determine filter input
	float input = p_cont->target_vel - measurement/dt;
	//Use Filter_Value to get the new filtered value filt_val of measurement
	float filt_val = Filter_Value(&(p_cont->controller), input);
	//Return the filtered measurement divided by dt to get the new control value
	return filt_val;
}

float Controller_Last(Controller_t* p_cont)
//Function Controller_Last returns the last control command
{
	//Pull the Filter_Data_t structure controller from p_cont
	//Use function Filter_Last_Output to get the most recent output from the controller
	//Return that most recent output
	return Filter_Last_Output(&(p_cont->controller));
}

void Controller_SetTo(Controller_t* p_cont, float measurement)
/*Function Controller_SettTo sets the Filter's input and output lists to match the
measurement so it starts with zero error*/
{
	//Use function Filter_SetTo to set the input and output lists initially equal to measurement
	Filter_SetTo(&(p_cont->controller), measurement);
	return;
}

void Controller_ShiftBy(Controller_t* p_cont, float measurement)
/*Function Controller_ShiftBy shifts the Filter's input and output lists by the desired
amount. This is helpful when dealing with wrapping*/
{
	//Use function Filter_ShiftBy to increment the input and output values by measurement
	Filter_ShiftBy(&(p_cont->controller), measurement);
	return;
}
