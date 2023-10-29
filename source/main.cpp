#include <iostream>
#include <sys/socket.h>
#include <ppu-types.h>

#include "../include/rendering.hpp"
#include "../include/pad.hpp"

s32 main(s32 argc, const char* argv[]) {	

	Renderer ren = Renderer();
	ren.rendering_loop();

	return 0;
}