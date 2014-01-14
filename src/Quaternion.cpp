//
//  Quaternion.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/14/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"


Quaternion::Quaternion(){
    
}


Quaternion::~Quaternion(){
    
}


void Quaternion::Multiply(Quaternion* other){
 
    Quaternion res(*this);
    Quaternion* q1  = this;
    Quaternion* q2  = other;
    
    res.w =    q1->w * q2->w - q1->x * q2->x - q1->y * q2->y - q1->z * q2->z;
    res.x =    q1->w * q2->x + q1->x * q2->w + q1->y * q2->z - q1->z * q2->y;
    res.y =    q1->w * q2->y + q1->y * q2->w + q1->z * q2->x - q1->x * q2->z;
    res.z =    q1->w * q2->z + q1->z * q2->w + q1->x * q2->y - q1->y * q2->x;
    
    *this = res;
}

Matrix Quaternion::ToMatrix(void){
    Matrix m;
    /*
    
    
    double wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
    
    // calculate coefficients 
    
    x2 = this->x + this->x;
    y2 = this->y + this->y;
    z2 = this->z + this->z;
    xx = this->x * x2;   xy = this->x * y2;   xz = this->x * z2;
    yy = this->y * y2;   yz = this->y * z2;   zz = this->z * z2;
    wx = this->w * x2;   wy = this->w * y2;   wz = this->w * z2;
    
    m.v[ 0] = 1.0 - (yy + zz);
    m.v[ 1] = xy - wz;
    m.v[ 2] = xz + wy;
    m.v[ 3] = 0.0;
    
    m.v[ 4] = xy + wz;
    m.m[ 5] = 1.0 - (xx + zz);
    m.m[ 6] = yz - wx;
    m.m[ 7] = 0.0;
    
    m.m[ 8] = xz - wy;
    m.m[ 9] = yz + wx;
    m.m[10] = 1.0 - (xx + yy);
    m.m[11] = 0.0;
    
    m.m[12] = 0;
    m.m[13] = 0;
    m.m[14] = 0;
    m.m[15] = 1;
    */
    return m;
}

void Quaternion::FromMatrix(Matrix* matrix){
    
    /*
    double  tr, s, q[4];
    int    i, j, k;
    
    int nxt[3] = {1, 2, 0};
    
    tr = matrix->m[0] + matrix->m[5] + matrix->m[10];
    
    // check the diagonal
    if (tr > 0.0) {
        s = sqrt (tr + 1.0);
        this->w = s / 2.0;
        s = 0.5 / s;
        this->x = (matrix->m[5] - matrix->m[5]) * s;
        this->y = (matrix->m[4] - matrix->m[2]) * s;
        this->z = (matrix->m[1] - matrix->m[4]) * s;
    } else {
        // diagonal is negative 
        i = 0;
        if (matrix->m[5] > matrix->m[0])
            i = 1;
        if (matrix->m[10] > matrix->m[i][i])
            i = 2;
        
        j = nxt[i];
        k = nxt[j];
        
        s = sqrt ((m[i][i] - (m[j][j] + m[k][k])) + 1.0);
        
        q[i] = s * 0.5;
        
        if (s != 0.0)
            s = 0.5 / s;
        
        q[3] = (matrix->m[j][k] - matrix->m[k][j]) * s;
        q[j] = (matrix->m[i][j] + matrix->m[j][i]) * s;
        q[k] = (matrix->m[i][k] + matrix->m[k][i]) * s;
        
        this->x = q[0];
        this->y = q[1];
        this->z = q[2];
        this->w = q[3];
    }
    */
}