//
//  Matrix.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

/*
 0  4  8 12
 1  5  9 13
 2  6 10 14
 3  7 11 15
 */

void Matrix::Print(void){
    
    for (size_t y=0; y < 4 ; y++) {
        for (size_t x=0; x < 4 ; x++) {
            printf("%.2f ",v[y][x]);
        }
        printf("\n");
    }
}


void Matrix::Clear(void){
    memset(v, 0, sizeof(v));
}

void Matrix::Identity(void){
    Clear();
    v[0][0] = 1;
    v[1][1] = 1;
    v[2][2] = 1;
    v[3][3] = 1;
}

void Matrix::PreMultiply(Matrix* other){
    
}


float* Matrix::ToGL(void){
    return (float*)this->v;
}
