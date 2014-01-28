//
//  SCStrike.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCStrike__
#define __libRealSpace__SCStrike__

class SCStrike : public IActivity{
    
public:
    SCStrike();
    ~SCStrike();
    
    void Init( );
    
    void RunFrame(void);

private:
    RSArea area ;
};

#endif /* defined(__libRealSpace__SCStrike__) */
