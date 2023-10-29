#include "../include/pad.h"

#include <ppu-types.h>
#include <sysutil/sysutil.h>
#include <io/pad.h>

/* 
	Normalize the analog stick axis.
	0 is default position and values range from -1 to 1 in realtive to the default position.
*/
f32 normalizeAnalogSticks(f32 raw) {
	f32 normalized = (raw - 128.f) / 128.f;
	return normalized;
}

/* 
	Fill the moveData struct with the data from the gamepad.
*/
void getControl(padData* pad, moveData* mov) {
	// Right stick movement
	// Get the stick values
	mov->vert_ang 	= M_PI * normalizeAnalogSticks((f32) pad->ANA_R_V);
	mov->horz_ang 	= M_PI * normalizeAnalogSticks((f32) pad->ANA_R_H);
	mov->distance	= 0.1  + normalizeAnalogSticks((f32) pad->ANA_L_V);
}
