//
//  Quaternion.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/14/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#include "Quaternion.h"


Quaternion::Quaternion(){
    w = 1;
    x = 0;
    y = 0;
    z = 0;
}


Quaternion::~Quaternion(){
    
}


void Quaternion::Multiply(Quaternion* other){
 
    Quaternion res;
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
    
    double wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;
    
    /* calculate coefficients */
    
    x2 = this->x + this->x;
    y2 = this->y + this->y;
    z2 = this->z + this->z;
    xx = this->x * x2;   xy = this->x * y2;   xz = this->x * z2;
    yy = this->y * y2;   yz = this->y * z2;   zz = this->z * z2;
    wx = this->w * x2;   wy = this->w * y2;   wz = this->w * z2;
    
    m.v[0][0] = 1.0 - (yy + zz);    m.v[0][1] = xy - wz;
    m.v[0][2] = xz + wy;            m.v[0][3] = 0.0;
    
    m.v[1][0] = xy + wz;            m.v[1][1] = 1.0 - (xx + zz);
    m.v[1][2] = yz - wx;            m.v[1][3] = 0.0;
    
    m.v[2][0] = xz - wy;            m.v[2][1] = yz + wx;
    m.v[2][2] = 1.0 - (xx + yy);    m.v[2][3] = 0.0;
    
    m.v[3][0] = 0;                  m.v[3][1] = 0;
    m.v[3][2] = 0;                  m.v[3][3] = 1;
    
    return m;
}

void Quaternion::FromMatrix(Matrix* m){
    
    double  tr, s, q[4];
    int    i, j, k;
    
    int nxt[3] = {1, 2, 0};
    
    tr = m->v[0][0] + m->v[1][1] + m->v[2][2];
    
    /* check the diagonal */
    if (tr > 0.0) {
        s = sqrt (tr + 1.0);
        this->w = s / 2.0;
        s = 0.5 / s;
        this->x = (m->v[2][1] - m->v[1][2]) * s;
        this->y = (m->v[0][2] - m->v[2][0]) * s;
        this->z = (m->v[1][0] - m->v[0][1]) * s;
    } else {
        /* diagonal is negative */
        i = 0;
        if (m->v[1][1] > m->v[0][0]) i = 1;
        if (m->v[2][2] > m->v[i][i]) i = 2;
        j = nxt[i];
        k = nxt[j];
        
        s = sqrt ((m->v[i][i] - (m->v[j][j] + m->v[k][k])) + 1.0);
        
        q[i] = s * 0.5;
        
        if (s != 0.0) s = 0.5 / s;
        
        q[3] = (m->v[k][j] - m->v[j][k]) * s;
        q[j] = (m->v[i][j] + m->v[j][i]) * s;
        q[k] = (m->v[i][k] + m->v[k][i]) * s;
        
        this->x = q[0];
        this->y = q[1];
        this->z = q[2];
        this->w = q[3];
    }
}


void Quaternion::Conjugate(void){
   // this->w =   this->w;
    this->x = - this->x;
    this->y = - this->y;
    this->z = - this->z;
}

float Quaternion::DotProduct(Quaternion* other){
    return(
           this->w * other->w +
           this->x * other->x +
           this->y * other->y +
           this->z * other->z);
}


Quaternion Quaternion::Slerp(Quaternion* other, float alpha ){
    double beta;			// complementary interp parameter
    double theta;			// angle between A and B
    double sin_t, cos_t;		// sine, cosine of theta
    double phi;				// theta plus spins
    int    bflip;			// use negation of B?
    
    int spin = 1;
    // cosine theta = dot product of A and B
    cos_t = this->DotProduct(other);//QuatDot(q1,q2);
    
    // if B is on opposite hemisphere from A, use -B instead
    if (cos_t < 0.0) {
        cos_t = -cos_t;
        bflip = 1;
    } else
        bflip = 0;
    
    /* if B is (within precision limits) the same as A,
     * just linear interpolate between A and B.
     * Can't do spins, since we don't know what direction to spin.
     */
    if (1.0 - cos_t < 1e-7) {
        beta = 1.0 - alpha;
    } else {				/* normal case */
        theta = acos(cos_t);
        phi   = theta + spin * M_PI;
        sin_t = sin(theta);
        beta  = sin(theta - alpha*phi) / sin_t;
        alpha = sin(alpha*phi) / sin_t;
    }
    
    if (bflip)
        alpha = -alpha;
    
    Quaternion qOut;
    
    /* interpolate */
    qOut.x = beta*this->x + alpha*other->x;
    qOut.y = beta*this->y + alpha*other->y;
    qOut.z = beta*this->z + alpha*other->z;
    qOut.w = beta*this->w + alpha*other->w;
    
    qOut.Normalize();
    //UnitQuat(qOut);
    
    return qOut;

}

static double MIN_NORM = 1.0e-7;

void Quaternion::Normalize(void){
    
    Quaternion *v  = this;
    
    double denom =
    sqrt( v->w*v->w + v->x*v->x + v->y*v->y + v->z*v->z );
    if(denom > MIN_NORM){ v->x /= denom;
        v->y /= denom;
        v->z /= denom;
        v->w /= denom; }
}