//
//  Camera.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

void gluLookAt(  Vector3D* vEye,  Vector3D* vLookat, Vector3D* vUp ,Matrix* viewMatrix)
{
    
    // determine the new n
    //vectorSubtract(vEye,vLookat,vN);
    Vector3D vN(*vEye);
    vN.Substract(vLookat);
    
    // determine the new u by crossing with the up vector
    //vectorCrossProduct(vUp, vN, vU) ;
    Vector3D vU = vUp->CrossProduct(&vN);
    
    // normalize both the u and n vectors
    //normalize(vU) ;
    //normalize(vN);
    vU.Normalize();
    vN.Normalize();
    
    
    // determine v by crossing n and u
    //vectorCrossProduct(vN,vU,vV);
    Vector3D vV = vN.CrossProduct(&vU);
    
    // create a model view matrix
    viewMatrix->m[0] = vU.x;     viewMatrix->m[4] = vU.y;  viewMatrix->m[8] = vU.z;  viewMatrix->m[12] = - vEye->DotProduct(&vU);
    viewMatrix->m[1] = vV.x;     viewMatrix->m[5] = vV.y;  viewMatrix->m[9] = vV.z;  viewMatrix->m[13] = - vEye->DotProduct(&vV);
    viewMatrix->m[2] = vN.x;     viewMatrix->m[6] = vN.y;  viewMatrix->m[10]= vN.z;  viewMatrix->m[14] = - vEye->DotProduct(&vN);
    viewMatrix->m[3]=   0.0f;     viewMatrix->m[7]= 0.0f;    viewMatrix->m[11]= 0.0f;   viewMatrix->m[15] = 1.0f;
    
}


void gluPerspective(float fovy, float aspect, float zNear, float zFar,Matrix* projectionMatrix)
{
    float f  = (float)(1 / tan(fovy*DEG_TO_RAD/2));
    
    
    projectionMatrix->m[0]= f/aspect;    projectionMatrix->m[4]= 0;  projectionMatrix->m[ 8]= 0;                         projectionMatrix->m[12]= 0;
    projectionMatrix->m[1]= 0;           projectionMatrix->m[5]= f;  projectionMatrix->m[ 9]= 0;                         projectionMatrix->m[13]= 0;
    projectionMatrix->m[2]= 0;           projectionMatrix->m[6]= 0;  projectionMatrix->m[10]=(zFar+zNear)/(zNear-zFar) ; projectionMatrix->m[14]= 2*(zFar*zNear)/(zNear-zFar);
    projectionMatrix->m[3]= 0;           projectionMatrix->m[7]= 0;   projectionMatrix->m[11]=-1;                        projectionMatrix->m[15]= 0;
}




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
    ::gluPerspective(fovy,aspect,zNear,zFar,projectionMatrix);
}

void Camera::gluLookAt(Matrix* viewMatrix){
    ::gluLookAt(&this->position,&this->lookAt, &this->up , viewMatrix);
}

void Camera::GetPosition(Point3D* position){
    this->position = *position;
}



