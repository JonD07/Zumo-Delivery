
#ifndef APPLICATION_DEFINES_H
#define APPLICATION_DEFINES_H

#define WHEEL_BASE		0.098
#define HALF_WHEEL_BASE	0.049

/** Message Driven State Machine Flags */
typedef struct MSG_FLAG { bool active; float duration; Time_t last_trigger_time; } MSG_FLAG_t;


MSG_FLAG_t mf_restart;       	///<-- This flag indicates that the device received a restart command from the hoast. Default inactive.
MSG_FLAG_t mf_loop_timer;    	///<-- Indicates if the system should report time to complete a loop.
MSG_FLAG_t mf_time_float_send;  ///<-- Indicates if the system should report the time to send a float.
MSG_FLAG_t mf_send_time;		///<-- Indicates if the system should send the current time.
MSG_FLAG_t mf_send_encoder;		///<-- Indicates if the system should send encoder counts.
MSG_FLAG_t mf_send_battery;		///<-- Indicates if the system should send the current battery level.
MSG_FLAG_t mf_low_battery;		///<-- Indicates if the system should send a low battery warning.
MSG_FLAG_t mf_battery_task;		///<-- Used to update the battery monitor
MSG_FLAG_t mf_timed_pwm;		///<-- For timed PWM operation
MSG_FLAG_t mf_sys_data;			///<-- Used to send periodic system status data
MSG_FLAG_t mf_motor_dist_control;	///<-- Enables motor controllers for distance
MSG_FLAG_t mf_motor_vel_control;	///<-- Enables motor controllers for velocity
MSG_FLAG_t mf_motor_stop;		///<-- Used to set PWM and control position & velocity to zero
MSG_FLAG_t mf_ir_proximity;		///<-- Used for IR proximity sensor
MSG_FLAG_t mf_obj_avoidance;		///<-- Used for object avoidance

#endif
