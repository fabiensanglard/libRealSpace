//
//  Math.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/13/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__Math__
#define __libRealSpace__Math__

typedef struct Point2D{
    int32_t x;
    int32_t y;
} Point2D;

typedef Point2D Vector2D;


// Long life to however came up with this. You rule man.
float InvSqrt(float x);

#endif /* defined(__libRealSpace__Math__) */
