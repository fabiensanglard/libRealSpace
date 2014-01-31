//
//  SCConvPlayer.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCConvPlayer__
#define __libRealSpace__SCConvPlayer__

class SCConvPlayer: public IActivity {
    
public :
    SCConvPlayer();
    ~SCConvPlayer();
    
    void Init( );
    
    void RunFrame(void);
    
    void   SetConvID(int32_t id);
    
private:
    
    size_t ReadConvFrame(ByteStream* reader);
    void    ParseConvArchive(PakEntry* conv);
    
};

#endif /* defined(__libRealSpace__SCConvPlayer__) */
