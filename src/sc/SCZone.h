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

/**
 * Structure representing a bounding box for a sprite.
 *
 * @struct sprtRect
 *
 * @var x1 The leftmost x-coordinate of the bounding box.
 * @var y1 The topmost y-coordinate of the bounding box.
 * @var x2 The rightmost x-coordinate of the bounding box.
 * @var y2 The bottommost y-coordinate of the bounding box.
 */
struct sprtRect {
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
};
/**
 * animatedSprites is a structure used to store information about animated sprites.
 *
 * - img: a pointer to a RSImageSet, which is a collection of images that can be used for animation.
 * - frames: a pointer to a vector of uint8_t, which stores the frames of the animation.
 * - frameCounter: a variable that keeps track of the current frame of the animation.
 * - rect: a pointer to a sprtRect, which stores the bounding box of the sprite.
 * - cliked: a boolean variable that stores whether the sprite has been clicked or not.
 * - quad: a pointer to a vector of Point2D, which stores the coordinates of the four corners of the sprite.
 * - efect: a pointer to a vector of EFCT, which stores the effects of the sprite.
 * - id: a variable that stores the ID of the sprite.
 * - shapid: a variable that stores the ID of the shape of the sprite.
 * - unkown: a variable that stores an unknown value.
 */
struct animatedSprites {
    RSImageSet *img{nullptr};
    std::vector<uint8_t> *frames{nullptr};
    uint8_t frameCounter{0};
    sprtRect *rect{nullptr};
    bool cliked{false};
    std::vector<Point2D *> *quad{nullptr};
    std::vector<EFCT *> *efect{nullptr};
    std::vector<REQU *> *requ{nullptr};
    uint8_t id{0};
    uint8_t shapid{0};
    uint8_t unkown{0};
    bool active{false};
};

class SCZone{
    
public:
    SCZone();
    ~SCZone();
    uint8_t id;
     
    Point2D position;
    Point2D dimension;
    std::vector<Point2D*> *quad;
    std::string *label;
    std::function<void(std::vector<EFCT *> *script, uint8_t id)> onclick;

    void OnAction(void);

    void Draw(void);
    
    inline bool IsEnabled(void){ return this->enabled; }
    inline void SetEnable(bool enabled){ this->enabled = enabled;}
    bool IsActive(std::map<uint8_t, bool> *requierd_flags);
    animatedSprites *sprite;
    
private:
    
    int fps;
    bool enabled ;
   

};

#endif /* defined(__libRealSpace__Zone__) */
