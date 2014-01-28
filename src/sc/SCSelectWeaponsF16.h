//
//  SCSelectWeaponsF16.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/26/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCSelectWeaponsF16__
#define __libRealSpace__SCSelectWeaponsF16__

//GAMEFLOW
// 70 74
//Main image 91	

class SCSelectWeaponF16 : public IActivity{
    
public:
    SCSelectWeaponF16();
    ~SCSelectWeaponF16();
    
    void Init( );
    
    void RunFrame(void);
    
private:
    
    RLEShape background;
};

#endif /* defined(__libRealSpace__SCSelectWeaponsF16__) */
