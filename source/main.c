#include <stdio.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/systime.h>
#include <sys/socket.h>
#include <ppu-types.h>

#include <tiny3d.h>
#include "../include/pad.h"
#include "../include/map.h"


/*
	To do:
	mit controller vom würfel weggehen mit der v axis
	h axis nach r/l bewegen
	Ist es richtig die rotation in model oder peojection zu machen?
	read map
*/

VECTOR eye = {1.0f, 1.0f, 1.0f};    // Camera's position
VECTOR center = {0.f, 0.f, 0.f}; 	// Point the camera is looking at
VECTOR up = {0.0f, 1.0f, 0.0f};     // Up direction

void render_pipeline(size_t index, padInfo* pad_info, padData* pad) {

	    sysUtilCheckCallback();
    	ioPadGetInfo(pad_info);
		
        if(pad_info->status)  {
			ioPadGetData(0, pad);

			// Dunno why it is used in all examples
			// MATRIX projectionMatrix = MatrixIdentity();
			// projectionMatrix = MatrixProjPerspective(90, 1920.0f / 1080.0f, 1.f, 1e3);
			
			MATRIX mat = MakeLookAtMatrix(eye, center, up);

			tiny3d_SetProjectionMatrix(&mat);

			moveData* mov;
			getControl(pad, mov);

			MATRIX rotZMat = MatrixRotationZ(mov->vert_ang);
			MATRIX rotYMat = MatrixRotationX(mov->vert_ang);
			MATRIX rotXMat = MatrixRotationY(mov->horz_ang);

			// Rotate first
			rotZMat 	= MatrixMultiply(rotZMat, MatrixTranslation(cube[index].x, cube[index].y, cube[index].z));
			rotZMat 	= MatrixMultiply(rotYMat, rotZMat);
			MATRIX MVP  = MatrixMultiply(rotXMat, rotZMat);

			// Scale according to the vertical position of the left stick 
			MATRIX scale = MatrixScale(mov->distance, mov->distance, mov->distance);
			MVP = MatrixMultiply(MVP, scale);

			tiny3d_SetMatrixModelView(&MVP);
		}
}


s32 main(s32 argc, const char* argv[]) {	
	int err = tiny3d_Init(TINY3D_Z16 | 4 * 1024 * 1024);
	if (err < 0) {
		perror("Couldnt init Tiny-3D");
		return EXIT_FAILURE;
	}

	padData gamepad;
	padInfo gamepad_info;
	ioPadInit(2);


	while (true) {
		tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);

        int err_poly = tiny3d_SetPolygon(TINY3D_POLYGON);
		if (err_poly == TINY3D_BUSY || err_poly == TINY3D_INVALID) {
			fprintf(stderr, "Error in setting up polygon %d\n", err_poly);
			break;
		}
		
		tiny3d_Project3D();

        for (size_t i = 0; i < sizeof(cube) / sizeof(vertices); i++) {
			render_pipeline(i, &gamepad_info, &gamepad);
			
			//cube[i].x += 5.f * normalizeAnalogSticks((f32) gamepad.ANA_L_H);	

			tiny3d_VertexPos(100 * cube[i].x, 100 * cube[i].y, 100 * cube[i].z);
            tiny3d_VertexColor(cube[i].color);
		}

		tiny3d_End();

		tiny3d_Flip();

	}

	return EXIT_SUCCESS;
}

// MVP.data[0][0] += 5.f * normalizeAnalogSticks((f32) pad->ANA_L_V);
// MVP.data[1][1] += 5.f * normalizeAnalogSticks((f32) pad->ANA_L_V);
// MVP.data[2][2] += 5.f * normalizeAnalogSticks((f32) pad->ANA_L_V);

//MVP.data[2][2] += normalizeAnalogSticks((f32) pad->ANA_L_V);
// // Vertical should scale 
// cube[index].y *= round(normalizeAnalogSticks((f32) pad->ANA_L_V));
// cube[index].x += round(normalizeAnalogSticks((f32) pad->ANA_L_H));