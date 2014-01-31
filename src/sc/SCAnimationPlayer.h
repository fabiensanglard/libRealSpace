//
//  SCAnimationPlayer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCAnimationPlayer__
#define __libRealSpace__SCAnimationPlayer__

class SCAnimationPlayer: public IActivity {
    public :
    SCAnimationPlayer(int32_t animationID, int32_t backgroundID);
    ~SCAnimationPlayer();
    
    void Init( );
    
    void RunFrame(void);
    
private:
    
};

#endif /* defined(__libRealSpace__SCAnimationPlayer__) */
