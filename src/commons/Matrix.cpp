//
//  Matrix.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "Matrix.h"

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
    
    /*
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
    */
    float result[4][4] = {
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0},
        {0,0,0,0}
    };

    /*
    *	matrix 4x4
        c00 = a00xb00+a01xb10+a02xb20+a03xb30
        c01 = a00xb10+a01xb11+a02xb12+a03xb13
        c02 = a00xb20+a01xb21+a02xb22+a03xb23
        c03 = a00xb30+a01xb31+a02xb32+a03xb33
    */

    for (int i = 0; i < 4; i++) {
        result[i][0] = other->v[i][0] * this->v[0][0] + other->v[i][1] * this->v[1][0] + other->v[i][2] * this->v[2][0] + other->v[i][3] * this->v[3][0];
        result[i][1] = other->v[i][0] * this->v[0][1] + other->v[i][1] * this->v[1][1] + other->v[i][2] * this->v[2][1] + other->v[i][3] * this->v[3][1];
        result[i][2] = other->v[i][0] * this->v[0][2] + other->v[i][1] * this->v[1][2] + other->v[i][2] * this->v[2][2] + other->v[i][3] * this->v[3][2];
        result[i][3] = other->v[i][0] * this->v[0][3] + other->v[i][1] * this->v[1][3] + other->v[i][2] * this->v[2][3] + other->v[i][3] * this->v[3][3];
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            this->v[i][j] = result[i][j];
        }
    }

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



void Matrix::SetTranslation(float x, float y, float z){
    v[3][0] = x;
    v[3][1] = y;
    v[3][2] = z;
}

void Matrix::SetRotationX(float angle){
    v[1][1] = cosf(angle);
    v[1][2] = sinf(angle);
    v[2][1] = -sinf(angle);
    v[2][2] = cosf(angle);
}

void Matrix::SetRotationY(float angle){
    v[0][0] = cosf(angle);
    v[0][2] = -sinf(angle);
    v[2][0] = sinf(angle);
    v[2][2] = cosf(angle);
}

void Matrix::SetRotationZ(float angle){
    v[0][0] = cosf(angle);
    v[0][1] = sinf(angle);
    v[1][0] = -sinf(angle);
    v[1][1] = cosf(angle);
}

void Matrix::translateM(float x, float y, float z) {
	Matrix transMatrix = { {
		{1,0,0,0},
		{0,1,0,0},
		{0,0,1,0},
		{x,y,z,1}
	}};
	this->Multiply(&transMatrix);
}

void Matrix::rotateM(float radangle, float x, float y, float z) {
	Matrix xrot = { { 
		{1					,0				,0					,0},
		{0					,cosf(radangle)	,sinf(radangle)		,0},
		{0					,-sinf(radangle),cosf(radangle)		,0},
		{0					,0				,0					,1}
	} };
    
	Matrix yrot = { {
		{cosf(radangle)		,0				,-sinf(radangle)	,0},
		{0					,1				,0					,0},
		{sinf(radangle)		,0				,cosf(radangle)		,0},
		{0					,0				,0					,1}
	}} ;
	Matrix zrot = {{
		{cosf(radangle)		,sinf(radangle)	,0					,0},
		{-sinf(radangle)	,cosf(radangle)	,0					,0},
		{0					,0				,1					,0},
		{0					,0				,0					,1},
	}};
	if (x != 0) {
		this->Multiply(&xrot);
	}
	if (y != 0) {
        this->Multiply(&yrot);
	}
	if (z != 0) {
        this->Multiply(&zrot);
	}
}
Vector3DHomogeneous Matrix::multiplyMatrixVector(Vector3DHomogeneous v) {  
    Vector3DHomogeneous result;
    
    result.x = this->v[0][0] * v.x + this->v[1][0] * v.y + this->v[2][0] * v.z + this->v[3][0] * v.w;
    result.y = this->v[0][1] * v.x + this->v[1][1] * v.y + this->v[2][1] * v.z + this->v[3][1] * v.w;
    result.z = this->v[0][2] * v.x + this->v[1][2] * v.y + this->v[2][2] * v.z + this->v[3][2] * v.w;
    result.w = this->v[0][3] * v.x + this->v[1][3] * v.y + this->v[2][3] * v.z + this->v[3][3] * v.w;
    
    return result;
}
void Matrix::Multiply(Vector3DHomogeneous other) {
    for (int i=0; i<4; i++) {
        this->v[i][0] = this->v[i][0] * other.x + this->v[i][0] * other.y + this->v[i][0] * other.z + this->v[i][0] * other.w;
        this->v[i][1] = this->v[i][1] * other.x + this->v[i][1] * other.y + this->v[i][1] * other.z + this->v[i][1] * other.w;
        this->v[i][2] = this->v[i][2] * other.x + this->v[i][2] * other.y + this->v[i][2] * other.z + this->v[i][2] * other.w;
        this->v[i][3] = this->v[i][3] * other.x + this->v[i][3] * other.y + this->v[i][3] * other.z + this->v[i][3] * other.w;
    }
}