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
            printf("% 4.2f ",v[y][x]);
        }
        printf("\n");
    }
}


void Matrix::Clear(void){
    for (size_t y=0; y < 4 ; y++) {
        for (size_t x=0; x < 4 ; x++) {
            v[y][x] = 0;
        }
    }
}

void Matrix::Identity(void){
    Clear();
    v[0][0] = 1;
    v[1][1] = 1;
    v[2][2] = 1;
    v[3][3] = 1;
}

void Matrix::Multiply(Matrix* other){
    
    /*                                         other
     
                                    [0][0] [1][0] [2][0] [3][0]
                                    [0][1] [1][1] [2][1] [3][1]
                                    [0][2] [1][2] [2][2] [3][2]
                                    [0][3] [1][3] [2][3] [3][3]
                this
     
     [0][0] [1][0] [2][0] [3][0]
     [0][1] [1][1] [2][1] [3][1]
     [0][2] [1][2] [2][2] [3][2]
     [0][3] [1][3] [2][3] [3][3]                   i  j
     */
    
    Matrix result;
    
    for(size_t i=0 ; i < 4 ; i ++){
        for(size_t j=0 ; j < 4 ; j ++){
            result.v[j][i] =
            v[0][i]  * other->v[j][0] +
            v[1][i]  * other->v[j][1] +
            v[2][i]  * other->v[j][2] +
            v[3][i]  * other->v[j][3] ;
        }
    }
                        
    *this = result;
}

void Matrix::Transpose(void){
    
    Matrix tmp;
    
    tmp.v[0][0] = v[0][0];
    tmp.v[1][1] = v[1][1];
    tmp.v[2][2] = v[2][2];
    tmp.v[3][3] = v[3][3];
    
    tmp.v[0][1] = v[1][0];
    tmp.v[0][2] = v[2][0];
    tmp.v[0][3] = v[3][0];
    
    tmp.v[1][0] = v[0][1];
    tmp.v[1][2] = v[2][1];
    tmp.v[1][3] = v[3][1];

    tmp.v[2][0] = v[0][2];
    tmp.v[2][1] = v[1][2];
    tmp.v[2][3] = v[3][2];

    tmp.v[3][0] = v[0][3];
    tmp.v[3][1] = v[1][3];
    tmp.v[3][2] = v[2][3];
    
    *this = tmp;
}


float* Matrix::ToGL(void){
    return (float*)this->v;
}
