#include "../include/pad.hpp"

#include <iostream>
/* 
	Normalize the analog stick axis.
	0 is default position and values range from -1 to 1 in realtive to the default position.
*/
f32 Pad::normalizeAnalogSticks(f32 raw) {
	f32 normalized = (raw - 128.f) / 128.f;
	return normalized;
}

/* 
	Fill the moveData struct with the data from the gamepad.
*/
void Pad::getControl(padData* pad, moveData* mov) {

	// Get the stick values
	mov->position_z_axis				+= normalizeAnalogSticks((f32) pad->ANA_L_V);
	// prevent mirroring at the x axis when crossing 0 
	if (mov->position_z_axis > 0) {
		mov->position_z_axis = 0.f;
	}

	mov->position_x_axis 				= normalizeAnalogSticks((f32) pad->ANA_L_H);

	//mov->directon_horizontal			+= normalizeAnalogSticks((f32) pad->ANA_R_);
}
