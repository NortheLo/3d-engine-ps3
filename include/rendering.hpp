#pragma once

#include <matrix.h>

#include "../include/pad.hpp"

class Renderer {
    public:
        moveData mov;
        Renderer();
        void rendering_loop();
        void render_pipeline(padInfo* pad_info, padData* pad);


    private:
        MATRIX modelView;
        Pad pad;
        VECTOR position = {0.f, 0.f, 0.f};
        VECTOR rotation = {0.f, 0.f, 0.f};
};