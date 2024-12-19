#pragma once
#include "defines.h"
#include <stdint.h>
#include <stdbool.h>

/* Acceleromter configuration */
#define ACCEL_ZERO_G	(0x80 << 2)
#define ACCEL_ONE_G	(0x9A << 2)

/* IR data modes */
#define IR_MODE_BASIC		1
#define IR_MODE_EXTENDED	3
#define IR_MODE_FULL		5

/* IR configuration */
#define IR_MAX_DOTS	4
#define IR_LOW_X	0x7F
#define IR_LOW_Y	0x5D
#define IR_HIGH_X	0x380
#define IR_HIGH_Y	0x2A2
#define IR_CENTER_X	((IR_HIGH_X + IR_LOW_X) >> 1)
#define IR_CENTER_Y	((IR_HIGH_Y + IR_LOW_Y) >> 1)
#define IR_HORIZONTAL_OFFSET	64
#define IR_VERTICAL_OFFSET	110
#define IR_DOT_SIZE	4
#define IR_DOT_CENTER_MIN_X (IR_LOW_X  + IR_HORIZONTAL_OFFSET)
#define IR_DOT_CENTER_MAX_X (IR_HIGH_X - IR_HORIZONTAL_OFFSET)
#define IR_DOT_CENTER_MIN_Y (IR_LOW_Y  + IR_VERTICAL_OFFSET)
#define IR_DOT_CENTER_MAX_Y (IR_HIGH_Y - IR_VERTICAL_OFFSET)
