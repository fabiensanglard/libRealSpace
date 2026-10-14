//
//  SCPilot.cpp
//  libRealSpace
//
//  Created by Rémi LEONARD on 23/09/2024.
//  Copyright (c) 2014 Fabien Sanglard. All rights reserved.
//
#include <algorithm>
#include "precomp.h"
#include "../engine/gametimer.h"
#include "SCMissionEvent.h"

SCPilot::SCPilot() {
    target_speed = 0;
    target_climb = 0;
    target_azimut = 0;
}

SCPilot::~SCPilot() {}

void SCPilot::SetGuidanceDirection(Vector3D direction) {
    this->attitude_mode = false;
    this->manual_throttle = -1;
    this->guidance_mode = GUIDANCE_DIRECTION;
    this->guidance_direction = direction;
}

void SCPilot::SetPitchCommand(float pitch_deg, float deadzone_deg) {
    this->attitude_mode = false;
    this->manual_throttle = -1;
    this->guidance_mode = GUIDANCE_PITCH;
    this->guidance_pitch = pitch_deg;
    this->guidance_deadzone = deadzone_deg;
}

void SCPilot::ClearGuidance() {
    this->guidance_mode = GUIDANCE_NONE;
    this->target_speed_ms = 0.0f;
}

void SCPilot::SetAttitudeError(float heading_error_deg, float pitch_error_deg, float deadband_deg) {
    this->guidance_mode = GUIDANCE_NONE;
    this->attitude_mode = true;
    this->attitude_heading_error = heading_error_deg;
    this->attitude_pitch_error = pitch_error_deg;
    this->attitude_deadband = deadband_deg;
}

void SCPilot::SetTargetWaypoint(Vector3D waypoint) {
    this->guidance_mode = GUIDANCE_NONE;
    this->attitude_mode = false;
    this->has_waypoint = true;
    this->target_waypoint = {
        waypoint.x, waypoint.y, waypoint.z
    };
    this->steer_waypoint = waypoint;
    Vector2D weapoint_direction = {
        waypoint.x - plane->x,
        waypoint.z - plane->z
    };
    float az = (atan2f((float)weapoint_direction.y, (float)weapoint_direction.x) * 180.0f / (float)M_PI);
    if (turning) {
        return;
    }
    //turning = true;
    az -= 360.0f;
    az += 90.0f;
    if (az > 360.0f) {
        az -= 360.0f;
    }
    while (az < 0.0f) {
        az += 360.0f;
    }
    this->target_azimut = az*10.0f;
    float ground_y = this->plane->area->getY(waypoint.x, waypoint.z);
    if (waypoint.y < ground_y) {
        waypoint.y = ground_y+1000;
    }
    float ground_level = this->plane->area->getY(this->plane->x, this->plane->z);
    if (this->plane->y < ground_y + 1000) {
        waypoint.y = ground_y + 1000;
    }
    target_climb = (int) waypoint.y;
    if (waypoint.y > plane->y || this->land) {
        
    } else {
        target_climb = (int) plane->y;
    }
}

float SCPilot::calculateMaxTurnRate(float airspeed, float maxG) {
    // Constantes
    const float G = 9.81f;          // Accélération gravitationnelle en m/s²
    const float KNOTS_TO_MS = 0.514444f;  // Facteur de conversion noeuds -> m/s
    
    // Vitesse en m/s
    float speed_ms = airspeed * KNOTS_TO_MS;
    
    // Calcul de l'angle de roulis maximum pour le facteur G donné
    float max_bank_angle_rad = acosf(1.0f / maxG);
    
    // Formule physique du taux de virage maximum (en degrés/seconde)
    float max_turn_rate_deg_sec = (G * tanf(max_bank_angle_rad)) / (M_PI * speed_ms / 180.0f);
    
    // Convertir en unités du jeu (dixièmes de degrés)
    return max_turn_rate_deg_sec * 10.0f;
}



/**
 * Retourne le delta time courant du moteur, avec une valeur de repli
 * cohérente avec la simulation si le timer n'est pas disponible.
 */
float SCPilot::getDeltaTime() {
    float dt = GameTimer::getInstance().getDeltaTime();
    if (dt <= 0.0f) {
        if (this->plane != nullptr && this->plane->tps > 0) {
            dt = 1.0f / this->plane->tps;
        } else {
            dt = 1.0f / 30.0f;
        }
    }
    return dt;
}

/**
 * Calcule l'angle de roulis maximum (en dixiemes de degres) que l'avion
 * peut soutenir pour son facteur de charge maximum.
 * bank = acos(1 / G)
 */
float SCPilot::maxBankForG(float maxG) {
    if (maxG <= 1.0f) {
        return 450.0f;
    }
    float bank_rad = acosf(1.0f / maxG);
    return radToDegree(bank_rad) * 10.0f;
}

/**
 * Gere les gaz pour atteindre la vitesse cible.
 * Rappel : vz est negatif quand l'avion avance, target_speed est negatif aussi.
 */
void SCPilot::controlThrottle() {
    if (this->target_speed_ms > 0.0f) {
        if (this->plane->worldVelocity().Length() < this->target_speed_ms) {
            this->throttle = 100;
        } else {
            this->throttle = (std::max)(0.0f, this->throttle - 10.0f);
        }
        return;
    }
    if (this->plane->forwardSpeedPerTick() > this->target_speed) {
        this->throttle = 100;
    } else {
        this->throttle = this->plane->GetThrottle() - 10;
        if (this->throttle < 0) {
            this->throttle = 0;
        }
    }
}

/**
 * Pilote automatique pilotant l'avion comme un humain : il n'agit QUE sur le
 * manche (control_stick_x / control_stick_y) et sur les gaz. La physique de
 * SCJdynPlane applique ensuite les forces reelles (portance, G, inertie, stall).
 *
 * Trois cascades de boucles :
 *   - lateral  : cap -> inclinaison voulue -> manche gauche/droite
 *   - vertical : altitude -> vitesse verticale voulue -> assiette -> manche cabrer/piquer
 *   - vitesse  : vitesse cible -> gaz
 *
 * La cascade verticale surveille la vitesse verticale reelle (vy) pour anticiper
 * l'inertie et eviter que l'avion n'oscille autour de l'altitude voulue.
 */
void SCPilot::FlyTo() {
    if (this->plane == nullptr) {
        return;
    }
    if (!this->alive) {
        return;
    }

    // Avion detruit : sequence de chute
    if (!this->plane->object->alive) {
        this->disengageAutopilot();
        this->target_speed = 0;
        this->target_climb = 0;
        this->target_azimut = 0;
        this->throttle = 0;
        this->control_stick_x = 0;
        this->control_stick_y = 0;
        this->publishControls();
        PlaneWreckEvent wreckEvent;
        wreckEvent.plane = this->plane;
        MessageBus::getInstance().publish(std::make_unique<PlaneWreckEvent>(wreckEvent));
        this->alive = false;
        return;
    }

    if (this->ground_ops) {
        this->throttle = (float) this->ground_throttle;
        this->control_stick_x = 0.0f;
        this->control_stick_y = this->ground_pitch_stick / 16.0f;
        this->publishControls(true);
        return;
    }
    if (!this->plane->on_ground && this->plane->GetWheel()) {
        this->gear = 0;
    }
    if (this->plane->on_ground && this->target_climb == 0) {
        this->throttle = 0;
        this->publishControls();
        return;
    }

    float dt = this->getDeltaTime();
    if (this->autopilotActive()) {
        this->runAutopilot(dt);
        return;
    }
    // en manoeuvre, le cran de gaz du bloc de commande (+0x1E) garde sa derniere valeur s'il n'est pas reecrit
    if (this->guidance_mode != GUIDANCE_MANUAL) {
        if (this->manual_throttle >= 0) {
            this->throttle = (float) this->manual_throttle;
        } else {
            this->controlThrottle();
        }
    }

    float roll_signed = signedRoll(this->plane->roll);

    // ============ PROTECTION ANTI-STALL (priorite absolue) ============
    if (this->plane->wing_stall > 0 && !this->plane->on_ground) {
        this->throttle = 100;
        this->control_stick_y = 80; // pousser pour reprendre de la vitesse
        // Remettre les ailes a plat (roll_signed -> 0).
        // control_stick_x > 0 fait DIMINUER roll_signed ; amortissement de meme signe que roll_speed.
        float recover_stick = 0.50f * roll_signed + 1.0f * this->plane->roll_speed;
        this->control_stick_x = (std::clamp)((int)recover_stick, -160, 160);
        this->publishControls();
        return;
    }
    if (this->guidance_mode == GUIDANCE_MANUAL) {
        if (this->manual_throttle >= 0) {
            this->throttle = (float) this->manual_throttle;
        }
        this->control_stick_x = this->roll_stick / 16.0f;
        this->control_stick_y = this->pitch_stick / 16.0f;
        this->publishControls(true);
        return;
    }
    if (this->guidance_mode != GUIDANCE_NONE) {
        this->runGuidance(dt);
        return;
    }
    if (!this->attitude_mode && !this->land && !this->plane->on_ground) {
        Vector3D direction;
        if (this->has_waypoint) {
            direction = Vector3D(this->steer_waypoint.x - this->plane->x, (float) this->target_climb - this->plane->y, this->steer_waypoint.z - this->plane->z);
        } else {
            float heading_yaw = tenthOfDegreeToRad(norm3600(3600.0f - this->target_azimut));
            direction = Vector3D(-sinf(heading_yaw) * 5000.0f, (float) this->target_climb - this->plane->y, -cosf(heading_yaw) * 5000.0f);
        }
        this->guidance_direction = direction;
        this->guidance_mode = GUIDANCE_DIRECTION;
        this->runGuidance(dt);
        this->guidance_mode = GUIDANCE_NONE;
        return;
    }

    // ============ LATERAL: cap -> angle de bank -> manche ============
    //
    // Conventions REELLES (verifiees) :
    //   control_stick_x > 0  => roule a DROITE => roll_signed DIMINUE (roll_speed < 0)
    //   control_stick_x < 0  => roule a GAUCHE => roll_signed AUGMENTE (roll_speed > 0)
    //   roll_signed : + = aile gauche basse, - = aile droite basse
    //   cap affiche = 360 - yaw/10  => virer a droite (cap+) <=> yaw diminue
    //
    float target_yaw  = norm3600(3600.0f - this->target_azimut);
    float heading_err = signed1800(target_yaw - this->plane->yaw);
    if (this->attitude_mode) {
        heading_err = -this->attitude_heading_error * 10.0f;
        if (fabsf(heading_err) <= this->attitude_deadband * 10.0f) {
            heading_err = 0.0f;
        }
    }

    float bank_limit = this->maxBankForG(this->plane->object->entity->jdyn->max_g);
    bank_limit = (std::clamp)(bank_limit, 250.0f, 600.0f);
    if (this->actor != nullptr &&
        this->actor->current_command == prog_op::OP_SET_OBJ_DEFEND_TARGET) {
        bank_limit = (std::clamp)(bank_limit * 1.25f, 250.0f, 900.0f);
    }

    // Angle de bank vise (convention roll_signed) : virer a droite => bank NEGATIF.
    const float HEADING_DEADZONE = 15.0f; // 1.5 deg : cap considere atteint

    // --- Serrage du virage : par le ROULIS (le tangage ne sert qu'a l'altitude) ---
    float turn_demand = (std::clamp)((fabsf(heading_err) - HEADING_DEADZONE) / 250.0f, 0.0f, 1.0f);
    float g_scale = this->plane->object->entity->jdyn->max_g / 10.0f;
    float skill   = (this->actor && this->actor->profile) ? (this->actor->profile->ai.atrb.AA / 16.0f) : 0.5f;

    // On incline davantage pour tourner plus fort (la portance sup. part a l'horizontale).
    float extra_bank = 250.0f * turn_demand * g_scale * (0.6f + 0.8f * skill);
    float hard_bank_limit = (std::clamp)(bank_limit + extra_bank, 250.0f, 850.0f);
    float bank_cmd = (std::clamp)(heading_err * 2.5f, -hard_bank_limit, hard_bank_limit);
    if (fabsf(heading_err) < HEADING_DEADZONE) {
        bank_cmd = 0.0f;
    }
    if (bank_cmd < -1.0f) {
        this->turnState = TURN_RIGHT;
    } else if (bank_cmd > 1.0f)  {
        this->turnState = TURN_LEFT;
    } else {
        this->turnState = TURN_NONE;
    }

    // bank_err > 0 => il faut AUGMENTER roll_signed (rouler a gauche) => stick NEGATIF.
    // D'ou le -Kp. Amortissement +Kd*roll_speed : un stick de meme signe que roll_speed
    // s'oppose a la rotation (roll_speed < 0 en roulant a droite => freine).
    float bank_err  = bank_cmd - roll_signed;
    float roll_stick = -0.50f * bank_err + 1.0f * this->plane->roll_speed;
    this->control_stick_x = (std::clamp)((int)roll_stick, -160, 160);

    // ============ VERTICAL: altitude -> vario -> assiette -> manche (inchange) ============
    const float pitch_stick_sign = 1.0f;
    float lead_time  = 0.70f;
    float lead_ticks = (std::clamp)(lead_time / dt, 4.0f, 24.0f);

    float predicted_y = this->plane->y + this->plane->vy * lead_ticks;
    float alt_err     = (float)this->target_climb - predicted_y;

    float vario_cmd = (std::clamp)(alt_err * 0.08f, -150.0f, 200.0f);
    float vario_err = vario_cmd - this->plane->vy;

    // Feed-forward de facteur de charge : tirer juste ce qu'il faut pour tenir
    // l'altitude a l'inclinaison courante => pas de montee parasite.
    float bank_rad       = tenthOfDegreeToRad(fabsf(roll_signed));
    float load_factor_ff = 1.0f / (std::max)(0.20f, cosf(bank_rad)) - 1.0f;
    float pitch_cmd = 1.35f * vario_err + load_factor_ff * 120.0f;
    pitch_cmd = (std::clamp)(pitch_cmd, -220.0f, 320.0f);
    if (this->attitude_mode) {
        float pitch_error = this->attitude_pitch_error * 10.0f;
        if (fabsf(pitch_error) <= this->attitude_deadband * 10.0f) {
            pitch_error = 0.0f;
        }
        pitch_cmd = (std::clamp)(this->plane->pitch + pitch_error, -450.0f, 450.0f) + load_factor_ff * 120.0f;
    }
    
    float pitch_err = pitch_cmd - this->plane->pitch;
    float desired_pitch_speed = 0.6f * pitch_err - 2.00f * this->plane->pitch_speed;

    float pitch_stick = pitch_stick_sign * desired_pitch_speed * 3.0f;
    this->control_stick_y = (std::clamp)((int)pitch_stick, -160, 160);

    this->publishControls();
}
void SCPilot::publishControls(bool normalized) {
    PlaneControlEvent planeControlEvent;
    planeControlEvent.normalized_stick = normalized;
    planeControlEvent.plane = this->plane;
    planeControlEvent.control_stick_x = this->control_stick_x;
    planeControlEvent.control_stick_y = this->control_stick_y;
    planeControlEvent.throttle = this->throttle;
    planeControlEvent.flaps = this->flap;
    planeControlEvent.spoilers = this->spoilers;
    planeControlEvent.wheel = this->gear;
    MessageBus::getInstance().publish(std::make_unique<PlaneControlEvent>(planeControlEvent));
}
void SCPilot::Fire(uint16_t weapon_mask, SCMissionActors *target) {
    for (size_t hardpoint = 0; hardpoint < this->plane->weaps_load.size(); hardpoint++) {
        SCWeaponLoadoutHardPoint *weap = this->plane->weaps_load[hardpoint];
        if (weap == nullptr || weap->nb_weap <= 0) {
            continue;
        }
        int weapon_id = weap->objct->wdat->weapon_id;
        if (weapon_id >= 1 && ((1 << (weapon_id - 1)) & weapon_mask) != 0) {
            PlaneFireEvent fireEvent;
            fireEvent.plane = this->plane;
            fireEvent.hardpoint = (int) hardpoint;
            fireEvent.target = target;
            fireEvent.mission = this->actor->mission;
            MessageBus::getInstance().publish(std::make_unique<PlaneFireEvent>(fireEvent));
            return;
        }
    }
}

void SCPilot::engageAutopilot(Vector3D target_point, Vector3D desired_velocity) {
    this->setAutopilotTarget(target_point, desired_velocity);
    this->autopilot_state = 0;
    this->autopilot_reached = false;
    float dt = this->getDeltaTime();
    this->autopilot_world_velocity = this->plane->worldVelocity();
    this->autopilot_hspeed = sqrtf(this->autopilot_world_velocity.x * this->autopilot_world_velocity.x + this->autopilot_world_velocity.z * this->autopilot_world_velocity.z);
}

void SCPilot::setAutopilotTarget(Vector3D target_point, Vector3D desired_velocity) {
    this->autopilot_point = target_point;
    this->autopilot_velocity = desired_velocity;
    this->autopilot_reached = false;
    this->target_waypoint = target_point;
}

void SCPilot::disengageAutopilot() {
    if (this->autopilot_state < 0) {
        return;
    }
    this->autopilot_state = -1;
    this->autopilot_reached = false;
    PlaneKinematicEvent event;
    event.plane = this->plane;
    event.engaged = false;
    MessageBus::getInstance().publish(std::make_unique<PlaneKinematicEvent>(event));
}

void SCPilot::runAutopilot(float dt) {
    Vector3D P = this->autopilot_point;
    Vector3D W = this->autopilot_velocity;
    Vector3D own(this->plane->x, this->plane->y, this->plane->z);
    float w_length = sqrtf(W.x * W.x + W.z * W.z);
    float w_heading = W.Azimuth();
    float R = w_length * 180.0f / (20.0f * (float) M_PI);
    float r = R - w_length * dt;
    Vector3D perp(W.z / w_length * r, 0.0f, -W.x / w_length * r);
    Vector3D right_center = P + perp;
    Vector3D left_center = P - perp;
    float d_left = sqrtf((left_center.x - own.x) * (left_center.x - own.x) + (left_center.z - own.z) * (left_center.z - own.z));
    float d_right = sqrtf((right_center.x - own.x) * (right_center.x - own.x) + (right_center.z - own.z) * (right_center.z - own.z));
    if (this->autopilot_state == 0) {
        this->autopilot_state = ((d_left < d_right && d_left > r) || d_right < r) ? 1 : 2;
    }
    Vector3D center = this->autopilot_state == 1 ? left_center : right_center;
    float d = this->autopilot_state == 1 ? d_left : d_right;

    float nose_heading = this->plane->forward.Azimuth();
    float error = 0.0f;
    if (d < R) {
        error = 0.0f;
    } else if (d < R + w_length * dt) {
        error = signed180(w_heading - nose_heading);
        if (this->autopilot_state == 1 && error > 0.0f) {
            error -= 360.0f;
        }
        if (this->autopilot_state == 2 && error < 0.0f) {
            error += 360.0f;
        }
    } else {
        float tangent = radToDegree(asinf(R / d));
        float aim = (center - own).Azimuth() + (this->autopilot_state == 1 ? tangent : -tangent);
        error = signed180(signed180(aim) - nose_heading);
    }
    float unclamped = error;
    error = (std::clamp)(error, -20.0f * dt, 20.0f * dt);
    float heading = nose_heading + error;

    Vector3D nose = this->plane->forward;
    float nose_pitch = this->nosePitch();
    Vector3D v = this->autopilot_world_velocity;
    float velocity_elevation = v.Elevation();
    float pitch_gap = velocity_elevation - nose_pitch;
    if (pitch_gap != 0.0f) {
        float f = (std::min)(1.0f, 5.0f * dt / std::fabs(pitch_gap));
        Vector3D level(nose.x, 0.0f, nose.z);
        level.Normalize();
        nose = nose + (level - nose) * f;
        nose.Normalize();
        nose_pitch = nose.Elevation();
    }

    float roll_deg = signed180(this->plane->roll / 10.0f);
    float roll_target = std::fabs(unclamped) > 10.0f ? (error > 0.0f ? 10.0f : -10.0f) : 0.0f;
    float roll_step = this->plane->object->entity->jdyn->max_turn_rate_dps * dt;
    roll_deg += (std::clamp)(roll_target - roll_deg, -roll_step, roll_step);

    float floor = this->plane->groundlevel + 250.0f;
    float target_y = P.y;
    if (target_y < floor) {
        target_y = own.y < floor ? floor : own.y;
    }
    float vertical_speed = (std::clamp)(target_y - own.y, -50.0f, 50.0f);

    float speed_gap = w_length - this->autopilot_hspeed;
    if (std::fabs(speed_gap) < 25.0f * dt) {
        this->autopilot_hspeed = w_length;
    } else {
        this->autopilot_hspeed += speed_gap > 0.0f ? 25.0f * dt : -25.0f * dt;
    }

    Vector3D horizontal(sinf(degreeToRad(heading)), 0.0f, cosf(degreeToRad(heading)));
    this->autopilot_world_velocity = horizontal * this->autopilot_hspeed + Vector3D(0.0f, vertical_speed, 0.0f);

    float reach_distance = 20.0f * w_length * (std::max)(dt, 0.2f);
    this->autopilot_reached = std::fabs(signed180(w_heading - heading)) < 5.0f && (P - own).Length() < reach_distance;

    PlaneKinematicEvent event;
    event.plane = this->plane;
    event.engaged = true;
    event.velocity = this->autopilot_world_velocity;
    event.yaw = norm3600((heading - 180.0f) * 10.0f);
    event.pitch = nose_pitch * 10.0f;
    event.roll = norm3600(roll_deg * 10.0f);
    MessageBus::getInstance().publish(std::make_unique<PlaneKinematicEvent>(event));
}

float SCPilot::bankAngle() {
    return -signedRoll(this->plane->roll) / 10.0f;
}

float SCPilot::nosePitch() {
    return radToDegree(asinf(std::clamp(this->plane->forward.y, -1.0f, 1.0f)));
}

void SCPilot::runGuidance(float dt) {
    this->roll_stick = 0.0f;
    this->pitch_stick = 0.0f;
    if (this->guidance_mode == GUIDANCE_DIRECTION) {
        this->guidanceSolution(this->guidance_direction, dt);
    } else {
        this->pitchToAngle(this->guidance_pitch, this->guidance_deadzone, dt);
    }
    this->control_stick_x = this->roll_stick / 16.0f;
    this->control_stick_y = this->pitch_stick / 16.0f;
    if (++this->guidance_log_counter >= 25) {
        this->guidance_log_counter = 0;
        printf("PILOT %s mode=%d h=%.1f v=%.1f r=%.1f bank=%.1f pitch=%.1f stick_x=%.2f stick_y=%.2f throttle=%.0f\n", this->actor != nullptr ? this->actor->actor_name.c_str() : "?", (int) this->guidance_mode, this->guidance_log_h, this->guidance_log_v, this->guidance_log_r, this->bankAngle(), this->nosePitch(), this->control_stick_x, this->control_stick_y, this->throttle);
    }
    this->publishControls(true);
}

// AI_GuidanceSolution_Major : reference = mon nez (AI_GuidanceCmd_FromOwnPos ; AI_Sensor_WeaponVelocityCache
// renvoie aussi le nez pour le canon et les missiles, loc_42E80 donnant un vecteur nul)
bool SCPilot::guidanceSolution(Vector3D direction, float dt) {
    Vector3D reference = this->plane->forward;
    float h = signed180(reference.Azimuth() - direction.Azimuth());
    float p = this->nosePitch();
    float e = std::fabs(h) >= 90.0f ? 0.0f : direction.Elevation();
    float deck = 200.0f;
    float floor = this->plane->groundlevel + deck;
    float altitude = this->plane->y;
    bool corrected = false;
    if (altitude <= floor && e < p) {
        e = (std::min)(80.0f, 80.0f * (floor - altitude) / deck);
        corrected = true;
    } else {
        float m = 0.0f;
        float n = (float) this->plane->object->entity->jdyn->max_g;
        if (n >= 2.0f) {
            float speed = this->plane->worldVelocity().Length();
            float radius = speed * speed / (9.8f * n / 2.0f);
            float above = altitude - floor;
            m = (radius <= 0.0f || above >= radius) ? -90.0f : -radToDegree(acosf((radius - above) / radius));
        }
        if (e < m) {
            e = m;
            corrected = true;
        } else if (p < -45.0f && e < p) {
            e = -45.0f;
            corrected = true;
        } else if (p >= 45.0f && e > p) {
            e = 45.0f;
            corrected = true;
        }
    }
    if (corrected) {
        float horizontal = sqrtf(direction.x * direction.x + direction.z * direction.z);
        direction.y = sinf(degreeToRad(e)) * horizontal;
    }
    float v = signed180(direction.Elevation() - reference.Elevation());
    float bank = this->bankAngle();
    float r = 0.0f;
    if (h >= 90.0f) {
        r = 90.0f - bank - (v > 0.0f ? v : 0.0f);
    } else if (h <= -90.0f) {
        r = -90.0f - bank + (v > 0.0f ? v : 0.0f);
    } else {
        r = this->BearingToRef(direction);
    }
    r = signed180(r);
    this->guidance_log_h = h;
    this->guidance_log_v = v;
    this->guidance_log_r = r;
    return this->combatDecision(h, v, r, dt);
}

bool SCPilot::combatDecision(float h, float v, float r, float dt) {
    bool too_slow = this->plane->wing_stall > 0 || this->plane->indicatedAirspeed() <= this->ReferenceSpeed();
    if (too_slow) {
        this->throttle = 100;
        v = (std::min)(v, 10.0f);
        h = (std::min)(h, 10.0f);
    }
    // l'original teste h == v == 0 sur des angles issus de tables d'arc indexees au 1/256 : nul sous asin(1/256) = 0,224 deg
    if (std::fabs(h) < 0.224f && std::fabs(v) < 0.224f) {
        this->rollToAngle(0.0f, 2.0f, dt);
        return true;
    }
    float a = sqrtf(h * h + v * v);
    if (a > 20.0f && v > -10.0f) {
        this->bankError(r, 2.0f, dt);
        if (std::fabs(r) < 20.0f) {
            this->throttle = 100;
            this->pitch_stick = this->clampPitch(16.0f);
        }
        return false;
    }
    float w = signed180(this->bankAngle() + r);
    if (std::fabs(w) > 145.0f) {
        float s = (std::max)(-16.0f, -16.0f * (v / 10.0f) * (v / 10.0f));
        if (this->rollToAngle(0.0f, 5.0f, dt)) {
            this->pitch_stick = this->clampPitch(s);
        }
        return false;
    }
    float k = (a / 20.0f) * (a / 20.0f) * this->plane->object->entity->jdyn->max_turn_rate_dps / 270.0f;
    float s = 16.0f * k;
    if (s >= 16.0f) {
        this->throttle = 100;
        s = 16.0f;
    }
    float t = r * k;
    if (std::fabs(t) > std::fabs(r)) {
        t = r;
    }
    this->guidance_log_t = t;
    this->guidance_log_s = s;
    if (this->bankError(t, 5.0f, dt)) {
        this->pitch_stick = this->clampPitch(s);
    }
    return false;
}

bool SCPilot::bankError(float error, float deadzone, float dt) {
    float g = (float) this->plane->object->entity->jdyn->max_g;
    float max_bank = g < 6.0f ? 90.0f * g / 6.0f : 90.0f;
    error = signed180(error);
    if (max_bank < 90.0f) {
        float bank = this->bankAngle();
        float wanted = (std::clamp)(bank + error, -max_bank, max_bank);
        error = wanted - bank;
    }
    if (std::fabs(error) > deadzone) {
        this->roll_stick = this->rollStickFromError(error, dt);
        return false;
    }
    return true;
}

bool SCPilot::rollToAngle(float bank, float deadzone, float dt) {
    float error = signed180(bank - this->bankAngle());
    if (std::fabs(error) > deadzone) {
        this->roll_stick = this->rollStickFromError(error, dt);
        return false;
    }
    return true;
}

bool SCPilot::pitchToAngle(float pitch, float deadzone, float dt) {
    float error = pitch - this->nosePitch();
    float bank = this->bankAngle();
    this->guidance_log_h = 0.0f;
    this->guidance_log_v = error;
    this->guidance_log_r = 0.0f;
    if (std::fabs(error) <= deadzone) {
        this->rollToAngle(0.0f, 5.0f, dt);
        return true;
    }
    if (error < -15.0f || (std::fabs(bank) > 90.0f && error < 0.0f)) {
        this->rollToAngle(180.0f, 5.0f, dt);
        if (std::fabs(bank) > 165.0f) {
            this->pitch_stick = this->clampPitch(16.0f * (std::max)(1.0f, std::fabs(error) / 15.0f));
        }
        return false;
    }
    this->rollToAngle(0.0f, 5.0f, dt);
    if (std::fabs(bank) < 15.0f) {
        this->pitch_stick = this->clampPitch(error < 15.0f ? 16.0f * error / 15.0f : 16.0f);
    }
    return false;
}

void SCPilot::BeginManual() {
    this->attitude_mode = false;
    this->guidance_mode = GUIDANCE_MANUAL;
    this->roll_stick = 0.0f;
    this->pitch_stick = 0.0f;
    this->manual_throttle = -1;
}

bool SCPilot::CmdRollTo(float bank_deg, float deadzone_deg) {
    return this->rollToAngle(bank_deg, deadzone_deg, 1.0f / 25.0f);
}

bool SCPilot::CmdPitchTo(float pitch_deg, float deadzone_deg) {
    return this->pitchToAngle(pitch_deg, deadzone_deg, 1.0f / 25.0f);
}

bool SCPilot::CmdGuidance(Vector3D direction) {
    return this->guidanceSolution(direction, 1.0f / 25.0f);
}

// Matrix_OrthonormalizeKeepRow1_57660 : nez garde, envergure = ancienne envergure projetee ; attitude en dixiemes de degre
void SCPilot::AttitudeFromAxes(Vector3D nose, Vector3D span, float &yaw, float &pitch, float &roll) {
    span = span - nose * span.DotProduct(&nose);
    span.Normalize();
    float yaw_deg = (-nose).Azimuth();
    float pitch_deg = nose.Elevation();
    Matrix zero_roll;
    zero_roll.Identity();
    zero_roll.rotateM(degreeToRad(yaw_deg), 0, 1, 0);
    zero_roll.rotateM(degreeToRad(pitch_deg), 1, 0, 0);
    Vector3D right0(zero_roll.v[0][0], zero_roll.v[0][1], zero_roll.v[0][2]);
    Vector3D up0(zero_roll.v[1][0], zero_roll.v[1][1], zero_roll.v[1][2]);
    float roll_deg = Vector2D(span.DotProduct(&right0), span.DotProduct(&up0)).Angle();
    yaw = norm3600(yaw_deg * 10.0f);
    pitch = pitch_deg * 10.0f;
    roll = norm3600(roll_deg * 10.0f);
}

void SCPilot::CmdPitchStick(float stick16) {
    this->pitch_stick = this->clampPitch(stick16);
}

void SCPilot::CmdRollStick(float stick16) {
    this->roll_stick = (std::clamp)(stick16, -16.0f, 16.0f);
}

void SCPilot::CmdThrottle(int notch) {
    this->manual_throttle = (std::clamp)(notch, 0, 10) * 10;
}

void SCPilot::BeginGroundOps() {
    this->ground_ops = true;
}

void SCPilot::EndGroundOps() {
    this->ground_ops = false;
}

void SCPilot::CmdGearUp() {
    this->gear = 0;
}

void SCPilot::CmdGroundControls(float pitch_stick16, int throttle_notch, int flaps, int gear, int spoilers) {
    this->ground_pitch_stick = (std::clamp)(pitch_stick16, -16.0f, 16.0f);
    this->ground_throttle = (throttle_notch < 0 || throttle_notch > 10) ? 0 : throttle_notch * 10;
    this->flap = flaps;
    this->gear = gear;
    this->spoilers = spoilers;
}

float SCPilot::YawOf(Vector3D heading_dir) {
    return norm3600((heading_dir.Azimuth() - 180.0f) * 10.0f);
}

void SCPilot::CmdKinematic(bool engaged, Vector3D velocity, Vector3D heading_dir, float pitch_deg) {
    PlaneKinematicEvent event;
    event.plane = this->plane;
    event.engaged = engaged;
    event.velocity = velocity;
    event.yaw = YawOf(heading_dir);
    event.pitch = pitch_deg * 10.0f;
    MessageBus::getInstance().publish(std::make_unique<PlaneKinematicEvent>(event));
}

void SCPilot::CmdPlaceAt(Vector3D position, Vector3D heading_dir, float pitch_deg) {
    PlaneKinematicEvent event;
    event.plane = this->plane;
    event.engaged = true;
    event.yaw = YawOf(heading_dir);
    event.pitch = pitch_deg * 10.0f;
    event.set_position = true;
    event.position = position;
    MessageBus::getInstance().publish(std::make_unique<PlaneKinematicEvent>(event));
}

float SCPilot::BankAngle() {
    return this->bankAngle();
}

float SCPilot::NosePitch() {
    return this->nosePitch();
}

// vitesse minimale de manoeuvre : la minimale, plus sin(assiette) * (croisiere - minimale) / 2 quand le nez monte
float SCPilot::ReferenceSpeed() {
    JDYN *jdyn = this->plane->object->entity->jdyn;
    float speed = (float) jdyn->ai_speed_min;
    float pitch = this->nosePitch();
    if (pitch > 0.0f) {
        speed += sinf(degreeToRad(pitch)) * (float) (jdyn->ai_speed_cruise - jdyn->ai_speed_min) / 2.0f;
    }
    return speed;
}

float SCPilot::BearingToRef(Vector3D direction) {
    Matrix &m = this->plane->ptw;
    float lx = direction.x * m.v[0][0] + direction.y * m.v[0][1] + direction.z * m.v[0][2];
    float ly = direction.x * m.v[1][0] + direction.y * m.v[1][1] + direction.z * m.v[1][2];
    return radToDegree(atan2f(lx, ly));
}

float SCPilot::rollStickFromError(float error, float dt) {
    float accel = this->plane->object->entity->jdyn->rate_limit_dps;
    float max_rate = this->maxRollRate(dt);
    if (max_rate == 0.0f) {
        return 0.0f;
    }
    float rate = sqrtf(accel * dt * accel * dt + 2.0f * accel * std::fabs(error)) - accel * dt;
    rate = (std::min)(rate, max_rate);
    return copysignf(16.0f * rate / max_rate, error);
}

float SCPilot::maxRollRate(float dt) {
    return this->plane->maxRollRate();
}

float SCPilot::clampPitch(float stick) {
    float flying = (float) std::max<int>(this->actor->profile->ai.atrb.FL, 8);
    float limit = 9.0f * flying / (float) this->plane->object->entity->jdyn->max_g;
    return (std::clamp)(stick, -limit, limit);
}
