//
//  SCZone.h
//  libRealSpace
//
//  Created by Rémi LEONARD on 22/08/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__Zone__
#define __libRealSpace__Zone__
#include <functional>

class SCZone{
    
public:
    SCZone();
    ~SCZone();
    uint8_t id;
     
    Point2D position;
    Point2D dimension;
    std::vector<Point2D*> *quad;
    std::string *label;
    std::function<void(uint8_t id)> onclick;

    void OnAction(void);

    void Draw(void);
    
    inline bool IsEnabled(void){ return this->enabled; }
    inline void SetEnable(bool enabled){ this->enabled = enabled;}
    
    
private:
    
    
    bool enabled ;
   

};

#endif /* defined(__libRealSpace__Zone__) */
