#pragma once

#include <stdio.h>
#include <ppu-types.h>

typedef struct {
	float x;
	float y;
	float z;
	u32 color;
} vertices;

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