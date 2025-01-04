#include "precomp.h"

bool SCMissionActors::execute() {
    return true;
}
bool SCMissionActors::takeOff(uint8_t arg) {
    if (this->pilot->target_climb == 0) {
        this->pilot->target_speed = -15;
        this->pilot->target_climb = this->plane->y + 300;
    }
    return (std::abs(this->plane->y-this->pilot->target_climb) < 10.0f);
}
bool SCMissionActors::land(uint8_t arg) {
    return true;
}
bool SCMissionActors::flyToWaypoint(uint8_t arg) {
    return true;
}
bool SCMissionActors::flyToArea(uint8_t arg) {
    return true;
}
bool SCMissionActors::destroyTarget(uint8_t arg) {
    Vector3D wp;
    
    for (auto actor: this->mission->actors) {
        if (actor->actor_id == arg) {
            if (!actor->plane->object->alive) {
                return true;
            }
            wp.x = actor->plane->x;
            wp.y = actor->plane->y;
            wp.z = actor->plane->z;
            this->pilot->SetTargetWaypoint(wp);
            Vector3D position = {this->plane->x, this->plane->y, this->plane->z};
            Vector3D diff = wp - position;
            float dist = diff.Length();
            if (!actor->plane->on_ground) {
                this->pilot->target_climb = (int) wp.y;
                if (dist > 1000.0f) {
                    this->pilot->target_speed = -60;
                } else if (dist > 300.0f) {
                    this->pilot->target_speed = (int) actor->plane->vz;
                }
            }
            return false;
        }
    }
}
bool SCMissionActors::defendTarget(uint8_t arg) {
    return true;
}
bool SCMissionActors::deactivate(uint8_t arg) {
    return true;
}
bool SCMissionActors::setMessage(uint8_t arg) {
    return true;
}
bool SCMissionActors::followAlly(uint8_t arg) {
    Vector3D wp;
    for (auto actor: this->mission->actors) {
        if (actor->actor_id == arg) {
            wp.x = actor->plane->x;
            wp.y = actor->plane->y;
            wp.z = actor->plane->z;
            this->pilot->SetTargetWaypoint(wp);
            Vector3D position = {this->plane->x, this->plane->y, this->plane->z};
            Vector3D diff = wp - position;
            float dist = diff.Length();
            if (!actor->plane->on_ground) {
                this->pilot->target_climb = (int) wp.y;
                if (dist > 1000.0f) {
                    this->pilot->target_speed = -60;
                } else if (dist > 300.0f) {
                    this->pilot->target_speed = (int) actor->plane->vz;
                }
            }
            
            return true;
        }
    }
}
bool SCMissionActors::ifTargetInSameArea(uint8_t arg) {
    Vector3D position = {this->plane->x, this->plane->y, this->plane->z};
    Uint8 area_id = this->mission->getAreaID(position);
    for (auto actor: this->mission->actors) {
        if (actor->actor_id == arg) {
            Uint8 target_area_id = this->mission->getAreaID({actor->plane->x, actor->plane->y, actor->plane->z});
            if (area_id == target_area_id) {
                return true;
            }
        }
    }
    return false;
}


bool SCMissionActorsPlayer::takeOff(uint8_t arg) {
    SCMissionWaypoint *waypoint = new SCMissionWaypoint();
    waypoint->spot = this->mission->mission->mission_data.spots[arg];
    waypoint->objective = new std::string("take off");
    this->mission->waypoints.push_back(waypoint);
    return true;
}
bool SCMissionActorsPlayer::land(uint8_t arg) {
    SCMissionWaypoint *waypoint = new SCMissionWaypoint();
    waypoint->spot = this->mission->mission->mission_data.spots[arg];
    waypoint->objective = new std::string("landing");
    this->mission->waypoints.push_back(waypoint);
    return true;
}
bool SCMissionActorsPlayer::flyToWaypoint(uint8_t arg) {
    SCMissionWaypoint *waypoint = new SCMissionWaypoint();
    waypoint->spot = this->mission->mission->mission_data.spots[arg];
    waypoint->objective = new std::string("Fly to\nWay Point");
    this->mission->waypoints.push_back(waypoint);
    return true;
}
bool SCMissionActorsPlayer::flyToArea(uint8_t arg) {
    SCMissionWaypoint *waypoint = new SCMissionWaypoint();
    waypoint->spot = this->mission->mission->mission_data.spots[arg];
    waypoint->objective = new std::string("Fly to\nWay Area");
    this->mission->waypoints.push_back(waypoint);
    return true;
}
bool SCMissionActorsPlayer::destroyTarget(uint8_t arg) {
    SCMissionWaypoint *waypoint = new SCMissionWaypoint();
    waypoint->spot = this->mission->mission->mission_data.spots[arg];
    waypoint->objective = new std::string("Destroy\nTarget");
    this->mission->waypoints.push_back(waypoint);
    return true;
}
bool SCMissionActorsPlayer::defendTarget(uint8_t arg) {
    SCMissionWaypoint *waypoint = new SCMissionWaypoint();
    waypoint->spot = this->mission->mission->mission_data.spots[arg];
    waypoint->objective = new std::string("Defend\nAlly");
    this->mission->waypoints.push_back(waypoint);
    return true;
}
/*
Fly to#Precise Way
Defend#Point
Follow#Leader
*/
bool SCMissionActorsPlayer::setMessage(uint8_t arg) {
    std::transform(this->mission->mission->mission_data.messages[arg]->begin(), this->mission->mission->mission_data.messages[arg]->end(), this->mission->mission->mission_data.messages[arg]->begin(), ::tolower);
    if (this->mission->waypoints.size() > 0) {
        this->mission->waypoints.back()->message = this->mission->mission->mission_data.messages[arg];
    }
    return true;
}