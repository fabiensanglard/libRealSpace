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
    VGAPalette palette;

public :
    RSCockpit* cockpit;
    SCCockpit();
    ~SCCockpit();
    void Init( );
    void Render(int face);
};
#endif