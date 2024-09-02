//
//  SCCockpit.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 02/09/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCCockpit__
#define __libRealSpace__SCCockpit__

#include "precomp.h"

class SCCockpit {
    RSCockpit* cockpit;

public :
    SCCockpit();
    ~SCCockpit();
    void Init( );
    void Render();
};
#endif