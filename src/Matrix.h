//
//  Matrix.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__Matrix__
#define __libRealSpace__Matrix__


#define DEG_TO_RAD (2.0f*M_PI/360.0f)

class Vector3D{
    
public:
    
    inline void SetWithCoo(float x, float y, float z){
        this->x = x;
        this->y = y;
        this->z = z;
    }
    
    inline void Clear(void){
        SetWithCoo(0, 0,0);
    }

    inline void Negate(void){
        Scale(-1);
    }
    
    inline void Add(Vector3D* other){
        this->x += other->x;
        this->y += other->y;
        this->z += other->z;
    }

    inline void Substract(Vector3D* other){
        this->x -= other->x;
        this->y -= other->y;
        this->z -= other->z;
    }
    

    inline void Scale(float factor){
        this->x *= factor;
        this->y *= factor;
        this->z *= factor;
    }

    
    inline Vector3D CrossProduct(const Vector3D* other){
        Vector3D result;

        result.x = this->y * other->z - this->z * other->y;                // X
        result.y = this->z * other->x - this->x * other->z;                // Y
        result.z = this->x * other->y - this->y * other->x;                // Z
        
        return result;
    }

    
    inline void Normalize(void){
        float ilength;
        
        ilength = DotProduct(this);
        
        float invSqrtLength = InvSqrt(ilength);
        
        Scale(invSqrtLength);
    }
    
    
    inline float DotProduct(Vector3D* other){
        
        float acc = 0;
        acc += this->x * other->x;
        acc += this->y * other->y;
        acc += this->z * other->z;
        
        return acc;
    }

    
    float x;
    float y;
    float z;
    
private:
};

#define Point3D Vector3D


class Matrix{
    public:
    
        void Clear(void);
        void Identity(void);
    
        void PreMultiply(Matrix* other);
    
        float m[16];
    
    private:
};





/*
typedef float vec3_t[3];
#define vectorClear( a )                ( (a)[ 0 ] = (a)[ 1 ] = (a)[ 2 ] = 0 )
#define vectorNegate( a, b )        ( (b)[ 0 ] = -(a)[ 0 ], (b)[ 1 ] = -(a)[ 1 ], (b)[ 2 ] = -(a)[ 2 ] )
#define vectorSet( v, x, y, z )        ( (v)[ 0 ] = ( x ), (v)[ 1 ] = ( y ), (v)[ 2 ] = ( z ) )
#define vectorInverse( a )                ( (a)[ 0 ] = (-a)[ 0 ], (a)[ 1 ] = (-a)[ 1 ], (a)[ 2 ] = (-a)[ 2 ] )
#define DotProduct( x, y )                        ( (x)[ 0 ] * (y)[ 0 ] + (x)[ 1 ] * (y)[ 1 ] + (x)[ 2 ] * (y)[ 2 ] )
#define vectorSubtract( a, b, c )        ( (c)[ 0 ] = (a)[ 0 ] - (b)[ 0 ], (c)[ 1 ] = (a)[ 1 ] - (b)[ 1 ], (c)[ 2 ] = (a)[ 2 ] - (b)[ 2 ] )
#define vectorAdd( a, b, c )                ( (c)[ 0 ] = (a)[ 0 ] + (b)[ 0 ], (c)[ 1 ] = (a)[ 1 ] + (b)[ 1 ], (c)[ 2 ] = (a)[ 2 ] + (b)[ 2 ] )
#define vectorCopy( b, a )                        ( (b)[ 0 ] = (a)[ 0 ], (b)[ 1 ] = (a)[ 1 ], (b)[ 2 ] = (a)[ 2 ] )
#define        vectorScale( v, s, o )                ( (o)[ 0 ] = (v)[ 0 ] * (s),(o)[ 1 ] = (v)[ 1 ] * (s), (o)[ 2 ] = (v)[ 2 ] * (s) )
#define        vectorMA( v, s, b, o )                ( (o)[ 0 ] = (v)[ 0 ] + (b)[ 0 ]*(s),(o)[ 1 ] = (v)[ 1 ] + (b)[ 1 ] * (s),(o)[ 2 ] = (v)[ 2 ] + (b)[ 2 ] * (s) )
#define vector_Interpolate( vtarget, v1, value, v2 ) ( (vtarget)[0] = (v1)[0] * (1 - (value)) + (v2)[0] * (value),   (vtarget)[1] = (v1)[1] * (1 - (value)) + (v2)[1] * (value)  , (vtarget)[2] = (v1)[2] * (1 - (value)) + (v2)[2] * (value)  )


void vectorCrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross );
float InvSqrt(float x);
void normalize(vec3_t v);
*/

#endif /* defined(__libRealSpace__Matrix__) */
