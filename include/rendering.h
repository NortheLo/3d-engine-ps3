#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ppu-types.h>

#include <tiny3d.h>
#include <matrix.h>

#include "../include/map.h"
#include "../include/pad.h"


VECTOR eye = {1.0f, 1.0f, 1.0f};    // Camera's position
VECTOR center = {0.f, 0.f, 0.f}; 	// Point the camera is looking at
VECTOR up = {0.0f, 1.0f, 0.0f};     // Up direction

void rendering_loop();

void render_pipeline(size_t index, padInfo* pad_info, padData* pad);