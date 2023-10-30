#pragma once

#include <matrix.h>

#include "../include/pad.hpp"

class Renderer {
    public:
        moveData mov;
        Renderer();

        void rendering_loop();
        void render_pipeline(size_t index, padInfo* pad_info, padData* pad);

    private:
        VECTOR eye = {1.0f, 1.0f, 1.0f};    // Camera's position
        VECTOR center = {0.f, 0.f, 0.f}; 	// Point the camera is looking at
        VECTOR up = {0.0f, 1.0f, 0.0f};     // Up direction
        Pad pad;
        
};