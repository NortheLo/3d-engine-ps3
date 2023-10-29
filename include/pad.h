#pragma once

// For reading the gamepad
#include <ppu-types.h>
#include <sysutil/sysutil.h>
#include <io/pad.h>

#define M_PI 3.14159265358979323846 

typedef struct {
    padData gamepad;
	padInfo gamepad_info;
	f32 vert_ang;
	f32 horz_ang;
	f32 distance;
} moveData;

/* 
	Normalize the analog stick axis.
	0 is default position and values range from -1 to 1 in realtive to the default position.
*/
f32 normalizeAnalogSticks(f32 raw);

/* 
	Fill the moveData struct with the data from the gamepad.
*/
void getControl(padData* pad, moveData* mov);

