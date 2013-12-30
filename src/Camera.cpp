//
//  Camera.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "precomp.h"

void gluLookAt(  vec3_t vEye,  vec3_t vLookat, vec3_t vUp ,matrix_t fModelView)
{
    vec3_t vN,vU,vV;
    
    // determine the new n
    vectorSubtract(vEye,vLookat,vN);
    
    // determine the new u by crossing with the up vector
    vectorCrossProduct(vUp, vN, vU) ;
    
    // normalize both the u and n vectors
    normalize(vU) ;
    normalize(vN);
    
    // determine v by crossing n and u
    vectorCrossProduct(vN,vU,vV);
    
    // create a model view matrix
    fModelView[0] = vU[0];                                        fModelView[4] = vU[1];                                        fModelView[8] = vU[2];                                        fModelView[12] = - DotProduct(vEye,vU);
    fModelView[1] = vV[0];                                        fModelView[5] = vV[1];                                        fModelView[9] = vV[2];                                        fModelView[13] = - DotProduct(vEye,vV);
    fModelView[2] = vN[0];                                        fModelView[6] = vN[1];                                        fModelView[10]= vN[2];                                        fModelView[14]=  - DotProduct(vEye,vN);
    fModelView[3]=        0.0f;                                        fModelView[7]= 0.0f;                                        fModelView[11]= 0.0f;                                        fModelView[15]= 1.0f;
    
}


void gluPerspective(float fovy, float aspect, float zNear, float zFar,matrix_t projectionMatrix)
{
    float f  = (float)(1 / tan(fovy*DEG_TO_RAD/2));
    
    
    projectionMatrix[0]= f/aspect;        projectionMatrix[4]= 0;        projectionMatrix[ 8]= 0;                                                                projectionMatrix[12]= 0;
    projectionMatrix[1]= 0;                 projectionMatrix[5]= f;        projectionMatrix[ 9]= 0;                                                                projectionMatrix[13]= 0;
    projectionMatrix[2]= 0;                        projectionMatrix[6]= 0;        projectionMatrix[10]=(zFar+zNear)/(zNear-zFar) ;                projectionMatrix[14]= 2*(zFar*zNear)/(zNear-zFar);
    projectionMatrix[3]= 0;                        projectionMatrix[7]=0;        projectionMatrix[11]=-1;                                                                projectionMatrix[15]= 0;
}




void Camera::Init(float fovy, float aspect, float zNear, float zFar){
    this->fovy = fovy;
    this->aspect = aspect;
    this->zNear = zNear;
    this->zFar =zFar;
}

void Camera::SetLookAt(vec3_t lookAt){
    vectorCopy(this->lookAt, lookAt);
}

void Camera::SetPosition(vec3_t position){
    vectorCopy(this->position, position);
}

void Camera::SetUp(vec3_t up){
    vectorCopy(this->up, up);
}

void Camera::gluPerspective(matrix_t projectionMatrix){
    ::gluPerspective(fovy,aspect,zNear,zFar,projectionMatrix);
}

void Camera::gluLookAt(matrix_t fModelView){
    ::gluLookAt(position,lookAt, up , fModelView);
}

void Camera::GetPosition(vec3_t position){
    vectorCopy(position, this->position);
}



