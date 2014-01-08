//
//  Event.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/7/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__Event__
#define __libRealSpace__Event__

class Event{
    
    uint32_t timestamp;
    uint32_t type;
    uint32_t args[4];
};

class Command{
    
public:
    
    void PumpSystem();
    
    bool HasMore(void);
    Event* GetNext();
};

#endif /* defined(__libRealSpace__Event__) */
