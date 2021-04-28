/*
 * Driver level macros
 */
#ifndef DRIVER_DEFINES_H
#define DRIVER_DEFINES_H

#include <ctype.h>
#include <stdbool.h>

#define PI		3.14159

typedef enum
{
	LEFT = 1,
	RIGHT
} eProximitySize;


typedef struct t_ProximityReturn {
	uint16_t m_nCount;
	eProximitySize m_eSide;
} t_ProximityReturn;

#endif
