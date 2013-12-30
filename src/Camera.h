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
    
    void SetLookAt(vec3_t lookAt);
    void SetPosition(vec3_t position);
    void SetUp(vec3_t up);
    
    void GetPosition(vec3_t position);
    
    void gluPerspective(matrix_t projectionMatrix);
    void gluLookAt(matrix_t fModelView);
    
private:
    
    float fovy;
    float aspect;
    float zNear;
    float zFar;
    
    vec3_t lookAt;
    vec3_t position;
    vec3_t up;
};

#endif /* defined(__libRealSpace__Camera__) */
