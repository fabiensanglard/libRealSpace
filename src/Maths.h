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

typedef Point2D Vector2D;


// Long life to however came up with this. You rule man.
float InvSqrt(float x);
float tenthOfDegreeToRad(float angle);
