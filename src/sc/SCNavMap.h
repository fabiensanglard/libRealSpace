#pragma once
//
//  NSObjectViewer.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 31/08/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCNavMap__
#define __libRealSpace__SCNavMap__

#include "precomp.h"

class SCNavMap : public IActivity{
    
public:
    SCNavMap();
    ~SCNavMap();
    
    void Init( );
    
    void RunFrame(void);
    
private:
    RSNavMap* navMap;
    
};

#endif /* defined(__libRealSpace__SCNavMap__) */
