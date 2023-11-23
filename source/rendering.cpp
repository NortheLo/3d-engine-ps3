#include "../include/rendering.hpp"

#include <iostream>
#include <cmath>
#include <memory>
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
		perror("Couldn't init Tiny-3D");
	}
}

/* 
	TO-DO:
	- fix position when going "off" grid
	- Think of a data structure to hold objects
	- How to render several objects
	- Load texture 
*/

void Renderer::rendering_loop() {

	padData gamepad;
	padInfo gamepad_info;
	ioPadInit(2);

	while (true) {
		tiny3d_Clear(0xff000000, TINY3D_CLEAR_ALL);

		render_pipeline(&gamepad_info, &gamepad);

        int err_poly = tiny3d_SetPolygon(TINY3D_POLYGON);
		if (err_poly == TINY3D_BUSY || err_poly == TINY3D_INVALID) {
			fprintf(stderr, "Error in setting up polygon %d\n", err_poly);
			break;
		}
		
		tiny3d_Project3D();

        for (auto &elem : cube) {
			
			tiny3d_VertexPos(elem.x, elem.y, elem.z);
            tiny3d_VertexColor(elem.color);
		}

		tiny3d_End();

		tiny3d_Flip();
	}
}

void Renderer::render_pipeline(padInfo* pad_info, padData* pad_data) {
	// Get data from the gamepad
	sysUtilCheckCallback();
	ioPadGetInfo(pad_info);

	if(pad_info->status)  {
		ioPadGetData(0, pad_data);
		pad.getControl(pad_data, &mov);

		MATRIX viewMatrix = MakeLookAtMatrix(pos, target, up);

		position.z += 1.f *  mov.position_z_axis;
		position.x -= 1.f * mov.position_x_axis;
		MATRIX modelMatrix = MatrixTranslation(position.x, 0, position.z);
		
		rotation.x += 0.1f * mov.pitch;
		rotation.y += 0.1f * mov.yaw;
		MATRIX yaw   = MatrixRotationY(rotation.y);
		MATRIX pitch = MatrixRotationX(rotation.x);

		modelMatrix = MatrixMultiply(modelMatrix, MatrixMultiply(yaw, pitch));
		MATRIX MVP = MatrixMultiply(modelMatrix, viewMatrix);

		//MVP = MatrixMultiply(projectionMatrix, MVP);


		tiny3d_SetProjectionMatrix(&projectionMatrix);
		tiny3d_SetMatrixModelView(&MVP);

	}
}
