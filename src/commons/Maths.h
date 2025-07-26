//
//  Math.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/13/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <math.h>
#include <stdint.h>

typedef struct Point2D{
    int32_t x;
    int32_t y;
} Point2D;

class Point2Df {
public:
    float x;
    float y;
    Point2Df(float x, float y) {
        this->x = x;
        this->y = y;
    };
    int operator==(const Point2Df& other) {
        return this->x == other.x && this->y == other.y;
    };
};
//typedef Point2D Vector2D;
class Vector2D {
public:
    float x;
    float y;
    Vector2D(float x, float y) {
        this->x = x;
        this->y = y;
    };
    Vector2D() {
        this->x = 0.0f;
        this->y = 0.0f;
    };
    int operator==(const Vector2D& other) {
        return this->x == other.x && this->y == other.y;
    };
    float Length() {
        return sqrtf((float)(this->x * this->x) + (float)(this->y * this->y));
    };
    void Normalize() {
        float len = this->Length();
        if (len > 0.0f) {
            this->x /= len;
            this->y /= len;
        }
    };
};

// Long life to however came up with this. You rule man.
float InvSqrt(float x);
float tenthOfDegreeToRad(float angle);
float degreeToRad(float angle);
float radToDegree(float angle);
