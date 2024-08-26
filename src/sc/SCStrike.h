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

private:
    
    Point3D* position;
    Camera *camera;
    float yaw;
    Point3D newPosition;
    Quaternion newOrientation;
    RSArea area ;
    RSMission *missionObj;
    SCPlane *playerplane;
    float counter;
    std::map<std::string, RSEntity*> objectCache;
};

#endif /* defined(__libRealSpace__SCStrike__) */
