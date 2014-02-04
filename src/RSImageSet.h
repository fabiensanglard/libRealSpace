//
//  RSImageSet.h
//  libRealSpace
//
//  Created by fabien sanglard on 2/3/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__RSImageSet__
#define __libRealSpace__RSImageSet__

class RSImageSet{
    
public:
    RSImageSet();
    ~RSImageSet();
    
    void InitFromPakEntry(PakEntry* entry);
    
    RLEShape* GetShape(size_t index);
    size_t GetNumImages(void);
    
    void Add(RLEShape* shape);
    
private:
    
    std::vector<RLEShape*> shapes;
};

#endif /* defined(__libRealSpace__RSImageSet__) */
