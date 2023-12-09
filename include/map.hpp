#pragma once


#include <ppu-types.h>

typedef struct {
	float x;
	float y;
	float z;
	u32 color;
} vertices;

typedef struct {
	float x;
	float y;
	float z;
} vertices2;

vertices cube[] = {
	// Front face
	{ -1.0f, -1.0f, -1.0f, 0xff000000 }, // Bottom left
	{ 1.0f, -1.0f, -1.0f, 0xff000000 },  // Bottom right
	{ 1.0f, 1.0f, -1.0f, 0xff000000 },   // Top right
	{ -1.0f, 1.0f, -1.0f, 0xff000000 },  // Top left

	// Back face
	{ -1.0f, -1.0f, 1.0f, 0xffff0000 },  // Bottom left
	{ 1.0f, -1.0f, 1.0f, 0xffff0000 },   // Bottom right
	{ 1.0f, 1.0f, 1.0f, 0xffff0000 },    // Top right
	{ -1.0f, 1.0f, 1.0f, 0xffff0000 },   // Top left

	// Left face
	{ -1.0f, -1.0f, -1.0f, 0xffffffff }, // Bottom front
	{ -1.0f, 1.0f, -1.0f, 0xffffffff },  // Top front
	{ -1.0f, 1.0f, 1.0f, 0xffffffff },   // Top back
	{ -1.0f, -1.0f, 1.0f, 0xffffffff },  // Bottom back

	// Right face
	{ 1.0f, -1.0f, -1.0f, 0x0f },  // Bottom front
	{ 1.0f, 1.0f, -1.0f, 0x0f },   // Top front
	{ 1.0f, 1.0f, 1.0f, 0x0f },    // Top back
	{ 1.0f, -1.0f, 1.0f, 0x0f },   // Bottom back

	// Top face
	{ -1.0f, 1.0f, -1.0f, 0xff00ff00 },  // Front left
	{ 1.0f, 1.0f, -1.0f, 0xff00ff00 },   // Front right
	{ 1.0f, 1.0f, 1.0f, 0xff00ff00 },    // Back right
	{ -1.0f, 1.0f, 1.0f, 0xff00ff00 },   // Back left

	// Bottom face
	{ -1.0f, -1.0f, -1.0f, 0xffff00ff }, // Front left
	{ 1.0f, -1.0f, -1.0f, 0xffff00ff },  // Front right
	{ 1.0f, -1.0f, 1.0f, 0xffff00ff },   // Back right
	{ -1.0f, -1.0f, 1.0f, 0xffff00ff }   // Back left
};

vertices2 cube2[] = {
    // Front face
    { 1.0f, -1.0f, -1.0f},  // Bottom left
    { 3.0f, -1.0f, -1.0f},  // Bottom right
    { 3.0f, 1.0f, -1.0f},   // Top right
    { 1.0f, 1.0f, -1.0f},   // Top left

    // Back face
    { 1.0f, -1.0f, 1.0f},   // Bottom left
    { 3.0f, -1.0f, 1.0f},   // Bottom right
    { 3.0f, 1.0f, 1.0f},    // Top right
    { 1.0f, 1.0f, 1.0f},    // Top left

    // Left face
    { 1.0f, -1.0f, -1.0f},  // Bottom front
    { 1.0f, 1.0f, -1.0f},   // Top front
    { 1.0f, 1.0f, 1.0f},    // Top back
    { 1.0f, -1.0f, 1.0f},   // Bottom back

    // Right face
    { 3.0f, -1.0f, -1.0f},  // Bottom front
    { 3.0f, 1.0f, -1.0f},   // Top front
    { 3.0f, 1.0f, 1.0f},    // Top back
    { 3.0f, -1.0f, 1.0f},   // Bottom back

    // Top face
    { 1.0f, 1.0f, -1.0f},   // Front left
    { 3.0f, 1.0f, -1.0f},   // Front right
    { 3.0f, 1.0f, 1.0f},    // Back right
    { 1.0f, 1.0f, 1.0f},    // Back left

    // Bottom face
    { 1.0f, -1.0f, -1.0f},  // Front left
    { 3.0f, -1.0f, -1.0f},  // Front right
    { 3.0f, -1.0f, 1.0f},   // Back right
    { 1.0f, -1.0f, 1.0f}    // Back left
};
