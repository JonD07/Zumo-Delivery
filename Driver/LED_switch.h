#ifndef LED_SWITCH_H
#define LED_SWITCH_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <ctype.h>

#include "driver_defines.h"

/*
 * LED_Init initializes the LED lights to turn on and off. We are
 * using pins PD5 for RED, PD0 for GREEN, and PD3 for BLUE
 */
void LED_Init();

/*
 * Set_LED turns LED led on or off based on boolean input on.
 */
void Set_LED(eLEDLights led, bool on);

#endif
