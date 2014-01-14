//
//  Camera.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__Camera__
#define __libRealSpace__Camera__

class Camera{

public:
    void Init(float fovy, float aspect, float zNear, float zFar);
    
    void SetLookAt(Point3D* lookAt);
    void SetPosition(Point3D* position);
    void SetUp(Point3D* up);
    
    void GetPosition(Point3D* position);
    
    void gluPerspective(Matrix* projectionMatrix);
    void gluLookAt(Matrix* fModelView);
    
   
private:
    
    float fovy;
    float aspect;
    float zNear;
    float zFar;
    
    Point3D lookAt;
    Point3D position;
    Point3D up;
    
    
};

#endif /* defined(__libRealSpace__Camera__) */
