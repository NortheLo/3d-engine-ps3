#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <ppu-types.h>

#include <tiny3d.h>

#include "../include/rendering.h"
#include "../include/pad.h"


s32 main(s32 argc, const char* argv[]) {	

	int err = tiny3d_Init(TINY3D_Z16 | 4 * 1024 * 1024);
	if (err < 0) {
		perror("Couldnt init Tiny-3D");
		return EXIT_FAILURE;
	}

	rendering_loop();

	return EXIT_SUCCESS;
}