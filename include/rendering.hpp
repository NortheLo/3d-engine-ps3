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
        Pad pad;
        VECTOR rotation = {0.f, 0.f, 0.f};
        VECTOR position = {0.f, 0.f, 0.f};

        VECTOR up = {0.f, 1.f, 0.f};
        VECTOR pos = {0.f, 0.f, -2.f};
        VECTOR target = {0.f, 0.f, 0.f};

        MATRIX projectionMatrix = MatrixProjPerspective(90.f, 1920.f / 1080.f, 0.00125, 300.f);
};

