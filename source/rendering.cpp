#include "../include/rendering.hpp"

#include <iostream>
#include <cmath>
#include <ppu-types.h>
#include <sys/socket.h>

#include <tiny3d.h>
#include <matrix.h>

#include "../include/pad.hpp"
#include "../include/map.hpp"
#include "../include/matrix2.h"

Renderer::Renderer() {
	int err = tiny3d_Init(TINY3D_Z16 | 4 * 1024 * 1024);
	if (err < 0) {
		perror("Couldnt init Tiny-3D");
	}

	// starting value
	mov.position_z_axis = 0.1;
}

/* 
	TO-DO:
	- Fix by constant l v stick position that the object keeps moving
	- Fix to see sides by going to the right/left
	- Fix yaw
*/

void Renderer::rendering_loop() {

	padData gamepad;
	padInfo gamepad_info;
	ioPadInit(2);
	f32 x_position_camera = 0.f;

	while (true) {
		x_position_camera += 0.1f * pad.normalizeAnalogSticks((f32) gamepad.ANA_L_H);

		tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);

        int err_poly = tiny3d_SetPolygon(TINY3D_POLYGON);
		if (err_poly == TINY3D_BUSY || err_poly == TINY3D_INVALID) {
			fprintf(stderr, "Error in setting up polygon %d\n", err_poly);
			break;
		}
		
		tiny3d_Project3D();
		render_pipeline(&gamepad_info, &gamepad);

        for (size_t i = 0; i < sizeof(cube) / sizeof(vertices); i++) {
			
			tiny3d_VertexPos(cube[i].x - x_position_camera, cube[i].y, cube[i].z);
            tiny3d_VertexColor(cube[i].color);
		}

		tiny3d_End();

		tiny3d_Flip();
	}
}

void Renderer::render_pipeline(padInfo* pad_info, padData* pad_data) {

	    sysUtilCheckCallback();
    	ioPadGetInfo(pad_info);

        if(pad_info->status)  {
			ioPadGetData(0, pad_data);
			pad.getControl(pad_data, &mov);

			cameraFront = Position(&cameraPos, &cameraFront);
			cameraFront = Direction(&cameraFront);
			
			MATRIX mat = MakeLookAtMatrix(cameraPos, cameraFront, up);
			tiny3d_SetProjectionMatrix(&mat);
			
			MATRIX MVP = Scale();

			tiny3d_SetMatrixModelView(&MVP);
		}
}

VECTOR Renderer::Position(VECTOR* cameraPosition, VECTOR* cameraTarget) {
	// For horizontal movement
	VectorCrossProduct(cameraTarget, &up);
	VectorNormalize(cameraTarget);
	VectorMultiply(cameraTarget, -1 * abs(mov.position_x_axis));
	VectorAdd(cameraTarget, cameraPosition);

	return *cameraTarget;
}

VECTOR Renderer::Direction(VECTOR* cameraFront) {
	// Pitch and yaw
	cameraFront->x = 2 * sin(mov.yaw) * (cos(mov.yaw) * cos(mov.pitch) - cos(mov.pitch));
	cameraFront->y = 			    -sin(mov.pitch);
	cameraFront->z = sin(mov.yaw) * cos(mov.pitch);
	VectorNormalize(cameraFront);

	return *cameraFront;
}

MATRIX Renderer::Scale() {
	// Scale according to the vertical position of the left stick 
	MATRIX scale = MatrixScale(mov.position_z_axis, mov.position_z_axis, mov.position_z_axis);
	MATRIX MVP = MatrixIdentity(); 
	MVP = MatrixMultiply(MVP, scale);
	return MVP;
}