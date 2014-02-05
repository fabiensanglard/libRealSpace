//
//  SCFont.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 2/5/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCFont__
#define __libRealSpace__SCFont__

class RSFont{
public:
    RSFont();
    ~RSFont();
    
    RLEShape* GetShapeForChar(char c);
    void InitFromPAK(PakArchive* fontArchive);
    
protected:
private:
   std::vector<RLEShape*> letters;
};

#endif /* defined(__libRealSpace__SCFont__) */
