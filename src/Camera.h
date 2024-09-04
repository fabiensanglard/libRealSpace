//
//  Camera.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#pragma once
#include <math.h>

#include "Maths.h"
#include "Matrix.h"
#include "Quaternion.h"


class Camera{

public:
    
    //Update parameters and recalculate perspective
    void SetPersective(float fovy, float aspect, float zNear, float zFar);
    
    //Update orientation and Matrix view
    void LookAt(Point3D* lookAt);
    
    //Update position and Matrix view
    void SetPosition(Point3D* position);
    
    //Update orientation and Matrix view
    void Rotate(float pitch, float yaw, float roll);
    void Camera::ResetRotate();
    void SetOrientation(Quaternion* orientation);
    /*
    void gluPerspective(Matrix* projectionMatrix);
    void gluLookAt(Matrix* fModelView);
    */
    
    //Various Getters
    Point3D GetPosition(void);
    Quaternion GetOrientation(void);
    Matrix* GetProjectionMatrix(void);
    Matrix* GetViewMatrix(void);
    
    
    
    //Moves
    void MoveForward(void);
    void MoveBackward(void);
    void MoveStrafLeft(void);
    void MoveStrafRight(void);
    
private:
    
    void CalcViewMatrix(void);
    
    float fovy;
    float aspect;
    float zNear;
    float zFar;
    
    Point3D position;
    Quaternion orientation;
    
    
    Matrix mproj;
    Matrix mview;
};

