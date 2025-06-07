//
//  Math.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/13/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "Maths.h"

float tenthOfDegreeToRad(float angle) { return (angle / 10) * (float(M_PI) / 180.0f); };
float degreeToRad(float angle) { return (angle) * (float(M_PI) / 180.0f); };
float radToDegree(float angle) { return (angle) * (180.0f / float(M_PI)); };
// Long life to however came up with this. You rule man.
float InvSqrt(float x)
{
    float xhalf = 0.5f*x;
    int i = *(int*)&x;        // get bits for floating value
    i = 0x5f3759df - (i>>1); // gives initial guess y0
    x = *(float*)&i;        // convert bits back to float
    x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
    return x;
}