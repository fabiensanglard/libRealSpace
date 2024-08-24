//
//  SCStrike.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//

#ifndef __libRealSpace__SCStrike__
#define __libRealSpace__SCStrike__
struct PlanePosition {
    float twist;
    float elevationf;
    float azimuthf;

    float x;
    float y;
    float z;
};

#define SC_WORLD 1;

class SCStrike : public IActivity{
    
public:
    SCStrike();
    ~SCStrike();
    
    void Init( );
    void SetMission(char* missionName);
    void CheckKeyboard(void);
    void RunFrame(void);
    float getY(float x, float z);

private:
    Point3D* position = new Point3D({0,0,0});
    Camera *camera;
    float yaw;
    Point3D newPosition;
    Quaternion newOrientation;
    PlanePosition *pp;
    RSArea area ;
    RSMission *missionObj;
    float counter;
    std::map<std::string, RSEntity*> objectCache;
};

#endif /* defined(__libRealSpace__SCStrike__) */
