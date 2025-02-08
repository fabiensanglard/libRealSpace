#pragma once
//
//  SCStrike.h
//  libRealSpace
//
//  Created by fabien sanglard on 1/28/2014.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#include "precomp.h"

#ifndef __libRealSpace__SCStrike__
#define __libRealSpace__SCStrike__

#define SCSTRIKE_MAX_MISSIONS 46
static const char *mission_list[] = {
    "MISN-1A.IFF",  "MISN-1B.IFF",  "MISN-1C.IFF",  "MISN-2A.IFF",  "MISN-3A.IFF",  "MISN-3B.IFF",  "MISN-3C.IFF",
    "MISN-4A.IFF",  "MISN-4B.IFF",  "MISN-4C.IFF",  "MISN-5A.IFF",  "MISN-5B.IFF",  "MISN-5BB.IFF", "MISN-5C.IFF",
    "MISN-6A.IFF",  "MISN-6B.IFF",  "MISN-6C.IFF",  "MISN-6D.IFF",  "MISN-7A.IFF",  "MISN-7B.IFF",  "MISN-7C.IFF",
    "MISN-7D.IFF",  "MISN-8A.IFF",  "MISN-8B.IFF",  "MISN-8C.IFF",  "MISN-9AA.IFF", "MISN-9A.IFF",  "MISN-9B.IFF",
    "MISN-10A.IFF", "MISN-10B.IFF", "MISN-10C.IFF", "MISN-10D.IFF", "MISN-11A.IFF", "MISN-11B.IFF", "MISN-11C.IFF",
    "MISN-12A.IFF", "MISN-12B.IFF", "MISN-13A.IFF", "MISN-13B.IFF", "MISN-14A.IFF", "MISN-14B.IFF", "MISN-15A.IFF",
    "MISN-3X.IFF",  "MISN-6X.IFF",  "MISN-8X.IFF",  "MISN-10X.IFF", "MISN-11X.IFF",
};
static int mission_idx = 0;

enum View { FRONT = 0, FOLLOW, RIGHT, LEFT, REAR, REAL, TARGET, EYE_ON_TARGET, MISSILE_CAM, OBJECT };


/**
 * @class SCStrike
 * @brief Implements the main game logic for Strike Commander
 *
 * This class implements the main game logic for Strike Commander. It handles
 * mission selection, keyboard input, game simulation, and rendering.
 */
class SCStrike : public IActivity {
private:
    /**
     * @brief Path to current mission file
     */
    std::string miss_file_name;
    /**
     * @brief Current camera mode
     */
    uint8_t camera_mode;
    /**
     * @brief Whether to control the plane with the mouse
     */
    bool mouse_control;
    /**
     * @brief Whether to pause the simulation
     */
    bool pause_simu{false};
    /**
     * @brief The main game camera
     */
    Camera *camera;
    /**
     * @brief The current position of the camera
     */
    Point3D camera_pos;
    /**
     * @brief The target position of the camera
     */
    Point3D target_camera_pos;
    /**
     * @brief The yaw of the camera
     */
    float yaw;
    /**
     * @brief The new position of the plane
     */
    Point3D newPosition;
    /**
     * @brief The current lookat position of the pilot
     */
    Point2D pilote_lookat;
    /**
     * @brief The new orientation of the plane
     */
    Quaternion newOrientation;
    /**
     * @brief The current area
     */
    RSArea area;
    /**
     * @brief The navigation screen
     */
    SCNavMap *nav_screen;
    /**
     * @brief The player plane
     */
    SCPlane *player_plane;

    /**
     * @brief The cockpit
     */
    SCCockpit *cockpit;

    /**
     * @brief Whether to use autopilot
     */
    bool autopilot{false};
    /**
     * @brief The pilot AI
     */
    SCPilot pilot;
    /**
     * @brief The player profile
     */
    RSProf *player_prof;
    /**
     * @brief A counter
     */
    float counter;
    /**
     * @brief The current navigation point
     */
    uint8_t nav_point_id{0};
    /**
     * @brief The current target
     */
    uint8_t current_target{0};
    /**
     * @brief A cache of objects
     */
    std::map<std::string, RSEntity *> objectCache;
    /**
     * @brief The AI planes
     */
    std::vector<SCAiPlane *> ai_planes;
    /**
     * @brief The MFD timeout
     */
    int32_t mfd_timeout{0};

    uint8_t current_object_to_view{0};

    SCMissionActors *target{nullptr};

    SCMission *current_mission{nullptr};
    int radio_mission_timer{0};
    bool show_bbox{false};
public:
    /**
     * @brief Constructor
     */
    SCStrike();
    /**
     * @brief Destructor
     */
    ~SCStrike();
    /**
     * @brief Initializes the game
     */
    void Init();
    /**
     * @brief Sets the current mission
     * @param missionName The name of the mission to load
     */
    void SetMission(char const *missionName);
    /**
     * @brief Checks for keyboard input
     */
    void CheckKeyboard(void);
    /**
     * @brief Runs the game simulation
     */
    void RunFrame(void);
    /**
     * @brief Renders the in-game menu
     */
    void RenderMenu();
};

#endif /* defined(__libRealSpace__SCStrike__) */
