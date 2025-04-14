//
//  Quaternion.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/14/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#pragma once

#include "Matrix.h"

class Quaternion{
    
    public:
    
     Quaternion();
    ~Quaternion();
    
    void Multiply(Quaternion* other);
    
    Matrix ToMatrix(void);
    
    void FromMatrix(Matrix* matrix);
    
    void Conjugate(void);
    
    float DotProduct(Quaternion* other);
    
    Quaternion Slerp(Quaternion* other, float alpha);
    
    void Normalize(void);
    void GetAngles(float& pitch, float& yaw, float& roll);
    void fromEulerAngles(float pitch, float roll);
            
    private:
    
        float w;
        float x;
        float y;
        float z;
    
};
