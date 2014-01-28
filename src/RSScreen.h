//
//  Screen.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/27/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__Screen__
#define __libRealSpace__Screen__

class RSScreen{
    
public:
    
    RSScreen();
    ~RSScreen();
    
    void Init(int32_t zoomFactor);
    void SetTitle(const char* title);
    void Refresh(void);
    
    int32_t width;
    int32_t height;
    int32_t scale;
private:
    
    
};

#endif /* defined(__libRealSpace__Screen__) */
