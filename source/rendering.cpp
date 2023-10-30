#include "../include/rendering.hpp"

#include <iostream>
#include <cmath>
#include <ppu-types.h>
#include <sys/socket.h>

#include <tiny3d.h>
#include <matrix.h>

#include "../include/pad.hpp"
#include "../include/map.hpp"


Renderer::Renderer() {
	int err = tiny3d_Init(TINY3D_Z16 | 4 * 1024 * 1024);
	if (err < 0) {
		perror("Couldnt init Tiny-3D");
	}

	// starting value
	mov.position_z_axis = 0.1;
}

void Renderer::rendering_loop() {

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

			tiny3d_VertexPos(cube[i].x, cube[i].y, cube[i].z);
            tiny3d_VertexColor(cube[i].color);
		}

		tiny3d_End();

		tiny3d_Flip();
	}
}

void Renderer::render_pipeline(size_t index, padInfo* pad_info, padData* pad_data) {

	    sysUtilCheckCallback();
    	ioPadGetInfo(pad_info);

        if(pad_info->status)  {
			ioPadGetData(0, pad_data);

			// Dunno why it is used in all examples
			// MATRIX projectionMatrix = MatrixIdentity();
			// projectionMatrix = MatrixProjPerspective(90, 1920.0f / 1080.0f, 1.f, 1e3);
			
			MATRIX mat = MakeLookAtMatrix(eye, center, up);

			tiny3d_SetProjectionMatrix(&mat);

			
			pad.getControl(pad_data, &mov);

			MATRIX xMatOffset = MatrixIdentity();
			xMatOffset.data[0][0] += mov.horz_ang;

			MATRIX rotZMat = MatrixRotationZ(mov.vert_ang);
			MATRIX rotYMat = MatrixRotationX(mov.vert_ang);
			MATRIX rotXMat = MatrixRotationY(mov.horz_ang);
			

			// Rotate first
			rotZMat 	= MatrixMultiply(rotZMat, MatrixTranslation(cube[index].x, cube[index].y, cube[index].z));
			rotZMat 	= MatrixMultiply(rotYMat, rotZMat);
			MATRIX MVP  = MatrixMultiply(rotXMat, rotZMat);

			// Scale according to the vertical position of the left stick 
			MATRIX scale = MatrixScale(mov.position_z_axis, mov.position_z_axis, mov.position_z_axis);
			MVP = MatrixMultiply(MVP, scale);

			tiny3d_SetMatrixModelView(&MVP);
		}
}