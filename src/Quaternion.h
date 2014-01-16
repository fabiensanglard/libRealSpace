//
//  Quaternion.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/14/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__Quaternion__
#define __libRealSpace__Quaternion__

class Quaternion{
    
    public:
    
     Quaternion();
    ~Quaternion();
    
    void Multiply(Quaternion* other);
    
    Matrix ToMatrix(void);
    
    void FromMatrix(Matrix* matrix);
    
    void Conjugate(void);
    
    float DotProduct(Quaternion* other);
    
    Quaternion Slerp(Quaternion* other, float alpha);
    
    void Normalize(void);
                     
    private:
    
        float w;
        float x;
        float y;
        float z;
    
};

#endif /* defined(__libRealSpace__Quaternion__) */
