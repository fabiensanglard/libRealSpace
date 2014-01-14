//
//  Camera.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"



void Camera::Init(float fovy, float aspect, float zNear, float zFar){
    this->fovy = fovy;
    this->aspect = aspect;
    this->zNear = zNear;
    this->zFar =zFar;
}

void Camera::SetLookAt(Point3D *lookAt){
    this->lookAt = *lookAt;
}

void Camera::SetPosition(Point3D* position){
    this->position = *position;
}

void Camera::SetUp(Vector3D* up){
    this->up = *up;
}

void Camera::gluPerspective(Matrix* projectionMatrix){
    float f  = (float)(1 / tan(fovy*DEG_TO_RAD/2));
    
    
    projectionMatrix->v[0][0]= f/aspect; projectionMatrix->v[1][0]= 0;  projectionMatrix->v[2][0]= 0;  projectionMatrix->v[3][0]= 0;
    projectionMatrix->v[0][1]= 0;        projectionMatrix->v[1][1]= f;  projectionMatrix->v[2][1]= 0;  projectionMatrix->v[3][1]= 0;
    projectionMatrix->v[0][2]= 0;        projectionMatrix->v[1][2]= 0;  projectionMatrix->v[2][2]=(zFar+zNear)/(zNear-zFar) ; projectionMatrix->v[3][2]= 2*(zFar*zNear)/(zNear-zFar);
    projectionMatrix->v[0][3]= 0;        projectionMatrix->v[1][3]= 0;   projectionMatrix->v[2][3]=-1;   projectionMatrix->v[3][3]= 0;
}

void Camera::gluLookAt(Matrix* viewMatrix){
    // determine the new n
    //vectorSubtract(vEye,vLookat,vN);
    Vector3D vN(this->position);
    vN.Substract(&this->lookAt);
    
    // determine the new u by crossing with the up vector
    //vectorCrossProduct(vUp, vN, vU) ;
    Vector3D vU = this->up.CrossProduct(&vN);
    
    // normalize both the u and n vectors
    //normalize(vU) ;
    //normalize(vN);
    vU.Normalize();
    vN.Normalize();
    
    
    // determine v by crossing n and u
    //vectorCrossProduct(vN,vU,vV);
    Vector3D vV = vN.CrossProduct(&vU);
    
    // create a model view matrix
    viewMatrix->v[0][0] = vU.x;  viewMatrix->v[1][0] = vU.y;  viewMatrix->v[2][0] = vU.z;  viewMatrix->v[3][0] = - position.DotProduct(&vU);
    viewMatrix->v[0][1] = vV.x;  viewMatrix->v[1][1] = vV.y;  viewMatrix->v[2][1] = vV.z;  viewMatrix->v[3][1] = - position.DotProduct(&vV);
    viewMatrix->v[0][2] = vN.x;  viewMatrix->v[1][2] = vN.y;  viewMatrix->v[2][2]= vN.z;   viewMatrix->v[3][2] = - position.DotProduct(&vN);
    viewMatrix->v[0][3]=   0.0f; viewMatrix->v[1][3]= 0.0f;   viewMatrix->v[2][3]= 0.0f;   viewMatrix->v[3][3] = 1.0f;
    

}

void Camera::GetPosition(Point3D* position){
    this->position = *position;
}



