//
//  RSMap.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 12/30/2013.
//  Copyright (c) 2013 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__RSMap__
#define __libRealSpace__RSMap__

class RSArea{
public:
    
    RSArea();
    ~RSArea();
    
    void InitFromPAKFileName(const char* pakFilename);
    
private:
    
    void ParseMetadata(void );
    void ParseObjects(void );
    void ParseTrigo(void );
    
    std::vector<RSMapTextureSet*> textures;
    PakArchive* archive;
};

#endif /* defined(__libRealSpace__RSMap__) */
