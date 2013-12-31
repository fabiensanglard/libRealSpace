//
//  RSMusic.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__RSMusic__
#define __libRealSpace__RSMusic__

class RSMusic{
    
public:
    void InitFromPAK(PakArchive* archive, uint32_t index);
    void InitFromRAM(uint8_t* data, size_t size);
    
};

#endif /* defined(__libRealSpace__RSMusic__) */
