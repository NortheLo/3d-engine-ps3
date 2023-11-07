#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <ppu-types.h>
#include <sysutil/sysutil.h>
#include <io/pad.h>

#define M_PI 3.14159265358979323846 

typedef struct {
    padData gamepad;
	padInfo gamepad_info;

	// Renaming all variable necessary
	f32 pitch;
	f32 yaw;
	f32 position_z_axis;
	f32 position_x_axis;
} moveData;

class Pad {
	private:
	
	public:
		/* 
		Normalize the analog stick axis.
		0 is default position and values range from -1 to 1 in realtive to the default position.
		*/
		f32 normalizeAnalogSticks(f32 raw);

		/* 
			Fill the moveData struct with the data from the gamepad.
		*/
		void getControl(padData* pad, moveData* mov);
};