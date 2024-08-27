#pragma once
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
    void SetMission(char* missionName);
    void CheckKeyboard(void);
    void RunFrame(void);
    void RenderMenu();
private:
    uint8_t camera_mode;
    bool mouse_control;
    Point3D* position;
    Camera *camera;
    Point3D camera_pos;
    float yaw;
    Point3D newPosition;
    Quaternion newOrientation;
    RSArea area ;
    RSMission *missionObj;
    SCPlane *player_plane;
    float counter;
    std::map<std::string, RSEntity*> objectCache;
};

#endif /* defined(__libRealSpace__SCStrike__) */
