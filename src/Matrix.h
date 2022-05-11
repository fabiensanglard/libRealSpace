//
//  Matrix.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
}
#endif
#define _USE_MATH_DEFINES
#include <math.h>

#include "Maths.h"

#define DEG_TO_RAD (2.0f*M_PI/360.0f)

class Vector3D{
    
public:
    
    inline void SetWithCoo(float x, float y, float z){
        this->x = x;
        this->y = y;
        this->z = z;
    }
    
    inline void Clear(void){
        SetWithCoo(0, 0,0);
    }

    inline void Negate(void){
        Scale(-1);
    }
    
    inline void Add(Vector3D* other){
        this->x += other->x;
        this->y += other->y;
        this->z += other->z;
    }

    inline void Substract(Vector3D* other){
        this->x -= other->x;
        this->y -= other->y;
        this->z -= other->z;
    }
    

    inline void Scale(float factor){
        this->x *= factor;
        this->y *= factor;
        this->z *= factor;
    }

    
    inline Vector3D CrossProduct(const Vector3D* other){
        Vector3D result;

        result.x = this->y * other->z - this->z * other->y;                // X
        result.y = this->z * other->x - this->x * other->z;                // Y
        result.z = this->x * other->y - this->y * other->x;                // Z
        
        return result;
    }

    
    inline void Normalize(void){
        float ilength;
        
        ilength = DotProduct(this);
        
        float invSqrtLength = InvSqrt(ilength);
        
        Scale(invSqrtLength);
    }
    
    
    inline float DotProduct(Vector3D* other){
        
        float acc = 0;
        acc += this->x * other->x;
        acc += this->y * other->y;
        acc += this->z * other->z;
        
        return acc;
    }

    
    float x;
    float y;
    float z;
    
private:
};

#define Point3D Vector3D


class Matrix{
    
public:
    
    void Clear(void);
    void Identity(void);
    
    void Multiply(Matrix* other);
    
    void Print(void);
    
    float* ToGL(void);
    
    void Transpose(void);
    
    float v[4][4];
    
    void SetTranslation(float x, float y, float z);
    void SetRotationX(float angle);
    void SetRotationY(float angle);
    void SetRotationZ(float angle);
    
private:
};

