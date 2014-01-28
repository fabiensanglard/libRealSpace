//
//  SCMouse.h
//  libRealSpace
//
//  Created by Fabien Sanglard on 1/25/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCMouse__
#define __libRealSpace__SCMouse__

class SCMouse{
    
public:
    SCMouse();
    ~SCMouse();
    
    void Init(void);
    
    inline bool IsVisible(void){ return this->visible ; }
    void SetVisible(bool visible){ this->visible = visible; }
    
    inline void SetPosition(Point2D position){ this->position = position;}
    inline Point2D GetPosition(void) { return this->position ; }
    
    void Draw(void);
    
private:

    enum AppearanceID {CURSOR, VISOR };
    RLEShape* appearances[4];
    AppearanceID mode;

    //The cursor position in VGA 320x200 coordinates.
    Point2D position;
    
    bool visible;
};

#endif /* defined(__libRealSpace__SCMouse__) */
