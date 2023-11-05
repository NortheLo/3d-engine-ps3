#pragma once

#include <math.h>
#include <matrix.h>

void VectorAdd(VECTOR* v1, VECTOR* v2) {
    v1->x += v2->x;
    v1->y += v2->y;
    v1->z += v2->z;
}

/* Multiplication with a scalar */
void VectorMultiply(VECTOR* v1, float scalar) {
    v1->x *= scalar;
    v1->y *= scalar;
    v1->z *= scalar;
}

void VectorNormalize(VECTOR* v1) {
    float mag = sqrt(v1->x * v1->x + 
                     v1->y * v1->y +
                     v1->z * v1->z);

    if (mag) {
        v1->x /= mag;
        v1->y /= mag;
        v1->z /= mag;
    }
}

void VectorCrossProduct(VECTOR* v1, VECTOR* v2) {
    VECTOR v3 = {0.f, 0.f, 0.f};

    v3.x = v1->y * v2->z - v1->z * v2->y;
    v3.y = v1->z * v2->x - v1->x * v2->z;
    v3.z = v1->x * v2->y - v1->y * v2->x;
	
    v1->x = v3.x;
    v1->y = v3.y;
    v1->z = v3.z;
}