//
//  SCMouse.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCMouse__
#define __libRealSpace__SCMouse__

class SCMouse{
    
public:
    SCMouse();
    ~SCMouse();
    
    void Init(void);
    
    bool IsVisible(void);
    
    RSImage* GetAppearance(void);
    
private:

    enum AppearanceID {INVISIBLE, CURSOR, VISOR };
    RSImage* appearances[2];
    AppearanceID currentMode;

    bool visible;
};

#endif /* defined(__libRealSpace__SCMouse__) */
