//
//  SCRegister.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/31/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCRegister__
#define __libRealSpace__SCRegister__

class SCRegister : public IActivity{
    
public:
    SCRegister();
    ~SCRegister();
    
    void Init( );
    
    void RunFrame(void);
    
private:
    RLEShape book;
};

#endif /* defined(__libRealSpace__SCRegister__) */
