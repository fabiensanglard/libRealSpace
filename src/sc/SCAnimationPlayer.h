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
    PakArchive *midgames;
    int current_img{0};
    int palette_id{0};
    int image_offset{0};
    public :
    SCAnimationPlayer();
    ~SCAnimationPlayer();
    void CheckKeyboard(void);
    void Init( );
    
    void RunFrame(void);
    
private:
    
};

#endif /* defined(__libRealSpace__SCAnimationPlayer__) */
