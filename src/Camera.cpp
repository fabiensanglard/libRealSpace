//
//  Camera.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "Camera.h"


void Camera::SetPersective(float fovy, float aspect, float zNear, float zFar){
    this->fovy = fovy;
    this->aspect = aspect;
    this->zNear = zNear;
    this->zFar =zFar;
    
    float f  = (float)(1 / tan(fovy*DEG_TO_RAD/2));
    
    
    mproj.v[0][0]= f/aspect; mproj.v[1][0]= 0;  mproj.v[2][0]= 0;                         mproj.v[3][0]= 0;
    mproj.v[0][1]= 0;        mproj.v[1][1]= f;  mproj.v[2][1]= 0;                         mproj.v[3][1]= 0;
    mproj.v[0][2]= 0;        mproj.v[1][2]= 0;  mproj.v[2][2]=(zFar+zNear)/(zNear-zFar) ; mproj.v[3][2]= 2*(zFar*zNear)/(zNear-zFar);
    mproj.v[0][3]= 0;        mproj.v[1][3]= 0;  mproj.v[2][3]=-1;                         mproj.v[3][3]= 0;

}

void Camera::LookAt(Point3D *lookAt){
    
    //Update orientation
    Vector3D vN(this->position);
    vN.Substract(lookAt);
    
    Vector3D up = {0,1,0};
    Vector3D vU = up.CrossProduct(&vN);
    
    vU.Normalize();
    vN.Normalize();
    
    // determine v by crossing n and u
    Vector3D vV = vN.CrossProduct(&vU);
    
    Matrix m;
    m.v[0][0] = vU.x;  m.v[1][0] = vU.y;  m.v[2][0] = vU.z;  m.v[3][0] = 0;
    m.v[0][1] = vV.x;  m.v[1][1] = vV.y;  m.v[2][1] = vV.z;  m.v[3][1] = 0;
    m.v[0][2] = vN.x;  m.v[1][2] = vN.y;  m.v[2][2]= vN.z;   m.v[3][2] = 0;
    m.v[0][3]=   0.0f; m.v[1][3]= 0.0f;   m.v[2][3]= 0.0f;   m.v[3][3] = 1.0f;
    
    orientation.FromMatrix(&m);
    /*
    printf("LookAt: injecting into Q:\n");
    m.Print();
    orientation.FromMatrix(&m);
    
    printf("LookAt: extractib from Q:\n");
    Matrix out = orientation.ToMatrix();
    out.Print();
    
    printf("LookAt: extractib from Q:\n");
    Matrix out2 = orientation.ToMatrix();
    out2.Transpose();
    out2.Print();
    */
    
    
    CalcViewMatrix();
}

void Camera::SetPosition(Point3D* position){
    this->position = *position;
    CalcViewMatrix();
}

void Camera::Rotate(float pitch, float yaw, float roll){
    
    /*
    Matrix rollRot;
    rollRot.Identity();
    rollRot.v[0][0] = cosf(yaw);
    rollRot.v[0][1] = sinf(yaw);
    rollRot.v[1][0] = -sinf(yaw);
    rollRot.v[1][1] = cosf(yaw);
    Quaternion qRollRot;
    qRollRot.FromMatrix(&rollRot);
    orientation.Multiply(&qRollRot);
    */
    
    
    //Generate all rotation quaternions.
    Matrix pitchRot;
    pitchRot.Identity();
    pitchRot.v[1][1] = cosf(pitch);
    pitchRot.v[1][2] = sinf(pitch);
    pitchRot.v[2][1] = -sinf(pitch);
    pitchRot.v[2][2] = cosf(pitch);
    Quaternion qPitchRot;
    qPitchRot.FromMatrix(&pitchRot);
    orientation.Multiply(&qPitchRot);
    
    
    
    
    Matrix yawRot;
    yawRot.Identity();
    yawRot.v[0][0] = cosf(yaw);
    yawRot.v[0][2] = -sinf(yaw);
    yawRot.v[2][0] = sinf(yaw);
    yawRot.v[2][2] = cosf(yaw);
    Quaternion qYawRot;
    qYawRot.FromMatrix(&yawRot);
    orientation.Multiply(&qYawRot);
    
    
    
    // Rotate orientation per Rotations Quat.
    
    
    CalcViewMatrix();
}

void Camera::CalcViewMatrix(void){
    
    /*
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
    */
    

    // Note: Instead of doing a matrix multiplication, it would
    //       save many cycles to just write the translation part
    //       in the rotation matrix.
    
    
    this->mview  = orientation.ToMatrix();
//    mview.Transpose();
    
    //Set the translation part.
    Matrix translation;
    translation.Identity();
    translation.v[3][0] = - position.x;
    translation.v[3][1] = - position.y;
    translation.v[3][2] = - position.z;
    
    
    
    this->mview.Multiply(&translation);
    
}

Point3D Camera::GetPosition(void){
    return this->position;
}


Quaternion Camera::GetOrientation(void){
    return this->orientation;
}

Matrix* Camera::GetProjectionMatrix(void){
    return &this->mproj;
}

Matrix* Camera::GetViewMatrix(void){
    return &this->mview;
}



void Camera::MoveForward(void){
    Matrix m = orientation.ToMatrix();
    this->position.x -= m.v[0][2]*1000;
    this->position.y -= m.v[1][2]*1000;
    this->position.z -= m.v[2][2]*1000;
    CalcViewMatrix();
}

void Camera::MoveBackward(void){
    Matrix m = orientation.ToMatrix();
    this->position.x += m.v[0][2]*1000;
    this->position.y += m.v[1][2]*1000;
    this->position.z += m.v[2][2]*1000;
    CalcViewMatrix();
}

void Camera::MoveStrafLeft(void){
    Matrix m = orientation.ToMatrix();
    this->position.x -= m.v[0][0]*1000;
    this->position.y -= m.v[1][0]*1000;
    this->position.z -= m.v[2][0]*1000;
    CalcViewMatrix();
}

void Camera::MoveStrafRight(void){
    Matrix m = orientation.ToMatrix();
    this->position.x += m.v[0][0]*1000;
    this->position.y += m.v[1][0]*1000;
    this->position.z += m.v[2][0]*1000;
    CalcViewMatrix();
}





