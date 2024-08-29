//
//  Camera.cpp
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#include "Camera.h"

void Camera::SetPersective(float fovy, float aspect, float zNear, float zFar) {
    this->fovy = fovy;
    this->aspect = aspect;
    this->zNear = zNear;
    this->zFar = zFar;

    float f = (float)(1 / tan(fovy * DEG_TO_RAD / 2));

    mproj.v[0][0] = f / aspect;
    mproj.v[1][0] = 0;
    mproj.v[2][0] = 0;
    mproj.v[3][0] = 0;
    mproj.v[0][1] = 0;
    mproj.v[1][1] = f;
    mproj.v[2][1] = 0;
    mproj.v[3][1] = 0;
    mproj.v[0][2] = 0;
    mproj.v[1][2] = 0;
    mproj.v[2][2] = (zFar + zNear) / (zNear - zFar);
    mproj.v[3][2] = 2 * (zFar * zNear) / (zNear - zFar);
    mproj.v[0][3] = 0;
    mproj.v[1][3] = 0;
    mproj.v[2][3] = -1;
    mproj.v[3][3] = 0;
}

void Camera::LookAt(Point3D *lookAt) {

    // Update orientation
    Vector3D vN(this->position);
    vN.Substract(lookAt);

    Vector3D up = {0, 1, 0};
    Vector3D vU = up.CrossProduct(&vN);

    vU.Normalize();
    vN.Normalize();

    // determine v by crossing n and u
    Vector3D vV = vN.CrossProduct(&vU);

    Matrix m;
    m.v[0][0] = vU.x;
    m.v[1][0] = vU.y;
    m.v[2][0] = vU.z;
    m.v[3][0] = 0;
    m.v[0][1] = vV.x;
    m.v[1][1] = vV.y;
    m.v[2][1] = vV.z;
    m.v[3][1] = 0;
    m.v[0][2] = vN.x;
    m.v[1][2] = vN.y;
    m.v[2][2] = vN.z;
    m.v[3][2] = 0;
    m.v[0][3] = 0.0f;
    m.v[1][3] = 0.0f;
    m.v[2][3] = 0.0f;
    m.v[3][3] = 1.0f;

    orientation.FromMatrix(&m);
    CalcViewMatrix();
}

void Camera::SetPosition(Point3D *position) {
    this->position = *position;
    CalcViewMatrix();
}

void Camera::SetOrientation(Quaternion *orientation) {
    this->orientation = *orientation;
    CalcViewMatrix();
}

void Camera::Rotate(float pitch, float yaw, float roll) {

    Matrix rollRot;
    rollRot.Identity();
    rollRot.rotateM(roll, 0, 0, 1);
    rollRot.rotateM(pitch, 1, 0, 0);
    rollRot.rotateM(yaw, 0, 1, 0);

    orientation.FromMatrix(&rollRot);
    CalcViewMatrix();
}

void Camera::CalcViewMatrix(void) {

    this->mview = orientation.ToMatrix();
    // Set the translation part.
    Matrix translation;
    translation.Identity();
    translation.v[3][0] = -position.x;
    translation.v[3][1] = -position.y;
    translation.v[3][2] = -position.z;

    this->mview.Multiply(&translation);
}

Point3D Camera::GetPosition(void) { return this->position; }

Quaternion Camera::GetOrientation(void) { return this->orientation; }

Matrix *Camera::GetProjectionMatrix(void) { return &this->mproj; }

Matrix *Camera::GetViewMatrix(void) { return &this->mview; }

void Camera::MoveForward(void) {
    Matrix m = orientation.ToMatrix();
    this->position.x -= m.v[0][2] * 100;
    this->position.y -= m.v[1][2] * 100;
    this->position.z -= m.v[2][2] * 100;
    CalcViewMatrix();
}

void Camera::MoveBackward(void) {
    Matrix m = orientation.ToMatrix();
    this->position.x += m.v[0][2] * 100;
    this->position.y += m.v[1][2] * 100;
    this->position.z += m.v[2][2] * 100;
    CalcViewMatrix();
}

void Camera::MoveStrafLeft(void) {
    Matrix m = orientation.ToMatrix();
    this->position.x -= m.v[0][0] * 100;
    this->position.y -= m.v[1][0] * 100;
    this->position.z -= m.v[2][0] * 100;
    CalcViewMatrix();
}

void Camera::MoveStrafRight(void) {
    Matrix m = orientation.ToMatrix();
    this->position.x += m.v[0][0] * 100;
    this->position.y += m.v[1][0] * 100;
    this->position.z += m.v[2][0] * 100;
    CalcViewMatrix();
}
