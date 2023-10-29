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
	{ 1.0f, -1.0f, -1.0f, 0xff0000ff },  // Bottom right
	{ 1.0f, 1.0f, -1.0f, 0xff00ff00 },   // Top right
	{ -1.0f, 1.0f, -1.0f, 0xff00ffff },  // Top left

	// Back face
	{ -1.0f, -1.0f, 1.0f, 0xffff0000 },  // Bottom left
	{ 1.0f, -1.0f, 1.0f, 0xffff00ff },   // Bottom right
	{ 1.0f, 1.0f, 1.0f, 0xffffff00 },    // Top right
	{ -1.0f, 1.0f, 1.0f, 0xffffffff },   // Top left

	// Left face
	{ -1.0f, -1.0f, -1.0f, 0xff000000 }, // Bottom front
	{ -1.0f, 1.0f, -1.0f, 0xff00ffff },  // Top front
	{ -1.0f, 1.0f, 1.0f, 0xffffffff },   // Top back
	{ -1.0f, -1.0f, 1.0f, 0xffff0000 },  // Bottom back

	// Right face
	{ 1.0f, -1.0f, -1.0f, 0xff0000ff },  // Bottom front
	{ 1.0f, 1.0f, -1.0f, 0xff00ff00 },   // Top front
	{ 1.0f, 1.0f, 1.0f, 0xffffff00 },    // Top back
	{ 1.0f, -1.0f, 1.0f, 0xffff00ff },   // Bottom back

	// Top face
	{ -1.0f, 1.0f, -1.0f, 0xff00ffff },  // Front left
	{ 1.0f, 1.0f, -1.0f, 0xff00ff00 },   // Front right
	{ 1.0f, 1.0f, 1.0f, 0xffffff00 },    // Back right
	{ -1.0f, 1.0f, 1.0f, 0xffffffff },   // Back left

	// Bottom face
	{ -1.0f, -1.0f, -1.0f, 0xff000000 }, // Front left
	{ 1.0f, -1.0f, -1.0f, 0xff0000ff },  // Front right
	{ 1.0f, -1.0f, 1.0f, 0xffff0000 },   // Back right
	{ -1.0f, -1.0f, 1.0f, 0xffff00ff }   // Back left
};