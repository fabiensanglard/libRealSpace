//
//  Matrix.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"



void vectorCrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross )
{
    cross[ 0 ] = v1[ 1 ] * v2[ 2 ] - v1[ 2 ] * v2[ 1 ];                // X
    cross[ 1 ] = v1[ 2 ] * v2[ 0 ] - v1[ 0 ] * v2[ 2 ];                // Y
    cross[ 2 ] = v1[ 0 ] * v2[ 1 ] - v1[ 1 ] * v2[ 0 ];                // Z
}

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

void normalize(vec3_t v)
{
    float ilength;
    //float length;
    //length = (float)sqrt( v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ] + v[ 2 ] * v[ 2 ] );
    
    ilength = InvSqrt(v[ 0 ] * v[ 0 ] + v[ 1 ] * v[ 1 ] + v[ 2 ] * v[ 2 ]);
    //        printf("Length = %.3f\n",length);
    
    //if( length )
    //{
    //ilength = 1 / length;
    v[ 0 ] *= ilength;
    v[ 1 ] *= ilength;
    v[ 2 ] *= ilength;
    //}
}