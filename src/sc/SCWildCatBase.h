//
//  WildCatBase.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__WildCatBase__
#define __libRealSpace__WildCatBase__

class SCWildCatBase : public IActivity {
    
public:
    SCWildCatBase();
    ~SCWildCatBase();
    
    void Init( );
    
    void RunFrame(void);
    
    void CheckKeyboard(void);
    
    RLEShape hangar;
    RLEShape vehicule;
};

#endif /* defined(__libRealSpace__WildCatBase__) */
