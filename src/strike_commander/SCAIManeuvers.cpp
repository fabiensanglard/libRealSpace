#include "precomp.h"
#include "SCAIBrain.h"

float SCAIBrain::referenceSpeed() {
    return owner->pilot->ReferenceSpeed();
}

// AI_InterceptSpeedControlLaw_5F9B : vitesse de la cible le long de mon nez (au moins la minimale) ; au-dela du seuil,
// vers la maximale selon la distance le long du nez (pleine a seuil + 3000 m) ; en deca, vers dword_72039
void SCAIBrain::interceptSpeed(SCMissionActors *target, float threshold) {
    JDYN *jdyn = owner->object->entity->jdyn;
    Vector3D nose = owner->plane->forward;
    Vector3D target_velocity = target->plane->worldVelocity();
    float closing = (std::max)(target_velocity.DotProduct(&nose), (float) jdyn->ai_speed_min);
    Vector3D delta = target->plane->position - owner->plane->position;
    float along = std::fabs(delta.DotProduct(&nose));
    float wanted;
    if (along > threshold) {
        float maximum = (float) jdyn->ai_speed_max;
        wanted = maximum > closing ? closing + (std::min)((along - threshold) / 3000.0f, 1.0f) * (maximum - closing) : maximum;
    } else {
        float reference = this->referenceSpeed();
        wanted = (closing - reference) * along / threshold + reference;
    }
    this->speedThrottle(wanted);
}

// AI_ManeuverSolution_Major : recalage de visee au canon ; W (direction de tir) = mon nez
bool SCAIBrain::gunSnap(SCMissionActors *target) {
    SCPilot *pilot = owner->pilot;
    Vector3D nose = owner->plane->forward;
    Vector3D direction = target->plane->position - owner->plane->position;
    float distance = direction.Length();
    direction.Normalize();
    float e = direction.Elevation() - nose.Elevation();
    float h = nose.Azimuth() - direction.Azimuth();
    float error = sqrtf(e * e + h * h);
    int roll_draw = 1 + std::rand() % 6 + std::rand() % 6 + std::rand() % 6;
    bool skilled = owner->profile->ai.atrb.AA >= roll_draw;
    float bearing = pilot->BearingToRef(direction);
    float window = (std::max)(7.0f * cosf(degreeToRad(std::fabs(bearing) / 4.0f)), 1.0f);
    if (!(std::fabs(h) < window && std::fabs(e) < window && distance < owner->mission->intel.range_gun && skilled)) {
        return error == 0.0f;
    }
    // nez <- nez + D - W (= D), Matrix_OrthonormalizeKeepRow1_57660
    Vector3D span(owner->plane->ptw.v[0][0], owner->plane->ptw.v[0][1], owner->plane->ptw.v[0][2]);
    PlaneAttitudeEvent event;
    event.plane = owner->plane;
    SCPilot::AttitudeFromAxes(direction, span, event.yaw, event.pitch, event.roll);
    MessageBus::getInstance().publish(std::make_unique<PlaneAttitudeEvent>(event));
    // roulis : vers la cible si |h| > fenetre / 2, sinon ailes a plat ; pas borne a JDYN+0x71 / 4 * dt
    float bank = std::fabs(h) > window / 2.0f ? bearing : -pilot->BankAngle();
    float step = (float) owner->object->entity->jdyn->max_turn_rate_dps / 4.0f * TICK_DURATION;
    if (std::fabs(bank) >= step) {
        bank = bank > 0.0f ? bank - step : bank + step;
    }
    pilot->CmdRollTo(bank, 5.0f);
    pilot->CmdPitchStick(0.0f);
    return true;
}

// AI_InterceptDispatcher_6F5C
bool SCAIBrain::interceptDispatcher(SCMissionActors *target) {
    this->interceptSpeed(target, owner->mission->intel.range_gun / 4.0f);
    if (this->gunSnap(target)) {
        return true;
    }
    return owner->pilot->CmdGuidance(target->plane->position - owner->plane->position);
}

float SCAIBrain::floorAltitude() {
    return owner->plane->groundlevel + 200.0f;
}

bool SCAIBrain::tooSlow() {
    return owner->plane->wing_stall > 0 || owner->plane->indicatedAirspeed() <= this->referenceSpeed();
}

bool SCAIBrain::tooLow() {
    return owner->plane->y < owner->plane->groundlevel + 4.0f * 200.0f;
}

int SCAIBrain::decisionWeight() {
    int value = owner->object->entity->jdyn->ai_decision_weight;
    if (value >= 2) {
        return (value - 2) * 3 / 2 + 3;
    }
    return -((2 - value) * 3 / 2 + 3);
}

void SCAIBrain::buildCombatContext(SCMissionActors *target) {
    JDYN *jdyn = owner->object->entity->jdyn;
    ctx.aircraft = owner->plane != nullptr;
    Vector3D own_velocity = owner->plane->worldVelocity();
    ctx.my_speed = own_velocity.Length();
    ctx.cruise = (float) jdyn->ai_speed_cruise;
    ctx.min_speed = (float) jdyn->ai_speed_min;
    ctx.ias = owner->plane->indicatedAirspeed();
    ctx.target = target;
    ctx.behind = false;
    ctx.head_on = false;
    if (target == nullptr || target->plane == nullptr) {
        ctx.dist = 0.0f;
        return;
    }
    Vector3D forward = owner->plane->forward;
    Vector3D target_velocity = target->plane->worldVelocity();
    ctx.D = target->plane->position - owner->plane->position;
    ctx.dist = ctx.D.Length();
    ctx.future_rel = (target->plane->position + target_velocity * 3.0f) - (owner->plane->position + own_velocity * 3.0f);
    ctx.nose_angle = forward.AngleBetween(ctx.D);
    ctx.future_angle = forward.AngleBetween(ctx.future_rel);
    ctx.target_vel_angle = ctx.D.AngleBetween(target_velocity);
    ctx.aspect = 180.0f - ctx.target_vel_angle;
    ctx.target_speed = target_velocity.Length();
    ctx.side = signed180(forward.Azimuth() - ctx.D.Azimuth());
    Vector3D direction = ctx.D;
    direction.Normalize();
    float los_speed = std::fabs(target_velocity.x * direction.x + target_velocity.y * direction.y + target_velocity.z * direction.z);
    bool close = ctx.dist < 3.0f * los_speed;
    if (ctx.nose_angle >= 100.0f && (ctx.future_angle <= 80.0f || (ctx.aspect < 50.0f && close))) {
        ctx.behind = true;
    } else if (ctx.nose_angle <= 60.0f && ctx.aspect < 60.0f && close) {
        ctx.head_on = true;
    }
}

int SCAIBrain::scoreManeuver(int id, SCMissionActors *target) {
    this->buildCombatContext(target);
    RSIntel &intel = owner->mission->intel;
    float range = intel.range_gun;
    int flying = owner->profile->ai.atrb.FL;
    int uses = maneuver.uses[id];
    bool has_target = target != nullptr && target->plane != nullptr;
    int score = 0;
    switch (id) {
        case 1:
        case 2: {
            if (!has_target || ctx.aspect > 20.0f || this->tooSlow() || this->tooLow()) {
                return 0;
            }
            score = 5 - uses / 2;
            if (id == 1) {
                if (ctx.aircraft && ctx.behind) {
                    return 0;
                }
                if (ctx.aspect < 30.0f && ctx.nose_angle > 90.0f) {
                    score += 2;
                    if (ctx.nose_angle > 135.0f && ctx.dist < range) {
                        score += 2;
                    }
                } else {
                    score -= ctx.aspect > 90.0f ? 10 : ctx.aspect > 45.0f ? 4 : 0;
                    if (ctx.nose_angle < 90.0f) {
                        score -= 3;
                    }
                    if (ctx.dist > range) {
                        score -= 3;
                    }
                }
                if (ctx.aircraft) {
                    score -= flying / 2;
                }
            } else {
                if (ctx.aircraft) {
                    return 0;
                }
                if (ctx.nose_angle > 135.0f && ctx.aspect < 30.0f && ctx.dist < range) {
                    score += 7;
                } else {
                    score -= ctx.aspect > 90.0f ? 10 : ctx.aspect > 45.0f ? 4 : 0;
                    if (ctx.nose_angle < 90.0f) {
                        score -= 3;
                    }
                    if (ctx.dist > range) {
                        score -= 3;
                    }
                }
                if (ctx.ias < ctx.min_speed) {
                    score -= 6;
                }
                if (this->skillCheck(owner->profile->ai.atrb.SM, 5)) {
                    score += 3;
                }
            }
            break;
        }
        case 3: {
            maneuver.phase_hint = 0;
            if (!has_target || ctx.aspect > 20.0f) {
                return 0;
            }
            score = 5;
            if (ctx.nose_angle < 90.0f) {
                score -= 3;
            }
            if (ctx.nose_angle > 135.0f && ctx.dist <= range) {
                score += 3;
            }
            if (ctx.dist > range) {
                score -= 3;
            }
            if (ctx.aircraft) {
                score -= flying / 4;
            }
            break;
        }
        case 4: {
            if (!has_target || ctx.aspect > 20.0f || this->tooSlow() || this->tooLow() || (ctx.aircraft && ctx.behind)) {
                return 0;
            }
            score = 5;
            if (ctx.nose_angle < 35.0f) {
                score += (ctx.future_angle > 135.0f && ctx.aspect < 45.0f) ? 4 : -10;
            } else if (ctx.aspect < 30.0f && ctx.nose_angle > 90.0f) {
                score += 2;
                if (ctx.nose_angle > 135.0f && ctx.dist < range) {
                    score += 2;
                }
            } else {
                if (ctx.aspect > 45.0f) {
                    score -= 2;
                }
                if (ctx.nose_angle < 90.0f) {
                    score -= 2;
                }
                if (ctx.dist > 2.0f * range) {
                    score -= 3;
                }
            }
            if (ctx.ias < ctx.min_speed) {
                score -= 8;
            } else if (ctx.ias < ctx.cruise) {
                score -= 2;
            }
            break;
        }
        case 5:
        case 6: {
            maneuver.phase_hint = 3;
            if (!has_target || !ctx.aircraft) {
                return 0;
            }
            if (id == 5) {
                if (this->tooSlow() || ctx.behind || ctx.ias < ctx.cruise) {
                    return 0;
                }
            } else {
                bool blocked = owner->plane->y < this->floorAltitude() + 4000.0f || ctx.behind;
                if (blocked) {
                    return 0;
                }
            }
            score = 3 - uses * 2;
            float dz = ctx.future_rel.y;
            if (id == 5) {
                if (dz < -2500.0f) {
                    score -= 7;
                } else if (dz >= 1250.0f) {
                    score += 2;
                }
            } else {
                if (dz > -2000.0f) {
                    score -= 7;
                } else {
                    score += 2;
                }
            }
            if (ctx.nose_angle < 35.0f) {
                score += (ctx.future_angle > 145.0f && ctx.aspect < 45.0f) ? 4 : -10;
            } else if (ctx.nose_angle > 90.0f && ctx.aspect < 45.0f) {
                if (ctx.dist < 2.0f * range) {
                    score += 7;
                } else {
                    score -= 2;
                }
            }
            if (id == 5) {
                if (ctx.my_speed < ctx.target_speed / 2.0f) {
                    score -= 4;
                } else if (ctx.my_speed < ctx.target_speed) {
                    score -= 2;
                } else if (ctx.my_speed > 2.0f * ctx.target_speed) {
                    score += 2;
                }
                score += this->decisionWeight();
                if (this->tooLow()) {
                    score += 4;
                } else if (owner->plane->y > (float) intel.unknown_range) {
                    score -= 6;
                    maneuver.phase_hint = 1;
                }
            } else {
                if (ctx.ias > ctx.cruise) {
                    score -= 4;
                    maneuver.phase_hint = 2;
                } else if (this->tooSlow()) {
                    score += 6;
                }
                if (ctx.my_speed > 2.0f * ctx.target_speed) {
                    score -= 6;
                } else if (ctx.my_speed > ctx.target_speed) {
                    score -= 3;
                } else if (ctx.my_speed < ctx.target_speed) {
                    score += 2;
                }
            }
            break;
        }
        case 7: {
            maneuver.bits = 0;
            if (owner->plane->wing_stall > 0 || !has_target) {
                return 0;
            }
            score = 5;
            if (flying > 13 || this->skillCheck(flying, -7)) {
                if (ctx.aspect < 90.0f) {
                    maneuver.bits |= 1;
                }
                if (ctx.nose_angle > 110.0f) {
                    maneuver.bits |= 1;
                    score -= 3;
                } else if (ctx.nose_angle < 50.0f) {
                    score += 3;
                }
                if (ctx.target_vel_angle < 90.0f) {
                    score += 4;
                }
                float elevation = ctx.D.Elevation();
                if (elevation > 45.0f) {
                    score -= (int) ((elevation - 20.0f) * 6.0f / 25.0f) + 4;
                    score -= this->tooSlow() ? 4 : 2;
                } else if (elevation < -30.0f) {
                    score -= 4;
                } else {
                    score += 4;
                }
                if (ctx.aircraft && ctx.behind) {
                    maneuver.point = ctx.future_rel;
                    maneuver.bits |= 2;
                    score += 10;
                } else if (ctx.aircraft && ctx.head_on) {
                    maneuver.point = ctx.future_rel;
                    maneuver.bits |= 8;
                    score += 10;
                }
            } else if ((ctx.nose_angle < 60.0f && ctx.target_vel_angle < 60.0f) || ctx.dist > range) {
                score += 2;
            } else {
                score -= 2;
            }
            return (std::max)(1, (std::min)(9, score));
        }
        case 13: {
            if (!has_target || this->tooSlow() || owner->plane->y >= owner->object->entity->jdyn->ai_engage_range) {
                return 0;
            }
            score = 1 - 2 * uses;
            bool not_far_below = ctx.D.y > -400.0f;
            if (ctx.target_speed < ctx.my_speed && not_far_below && ctx.my_speed >= (ctx.cruise + ctx.min_speed) / 2.0f && ctx.aircraft && (ctx.behind || ctx.head_on)) {
                if (ctx.target_speed <= ctx.min_speed) {
                    score += 5;
                } else {
                    score += (int) (5.0f * (ctx.my_speed - ctx.target_speed) / ctx.target_speed);
                }
            }
            if (ctx.dist > (float) intel.range_long && ctx.aspect < 30.0f) {
                score += 5;
            } else if (ctx.dist < range) {
                score -= 5;
                if (ctx.aspect < 20.0f) {
                    score -= 5;
                }
            }
            score += this->decisionWeight();
            if (this->tooLow()) {
                score += 4;
            }
            break;
        }
        case 14: {
            if (has_target || owner->pilot->autopilotActive()) {
                return 0;
            }
            float clear = (std::min)(9.0f * (float) (std::max)(flying, 8) / 32.0f, 5.0f);
            float height = owner->plane->y - owner->plane->groundlevel;
            float roll = owner->pilot->BankAngle();
            float vertical_speed = owner->plane->worldVelocity().y;
            if (height < clear * (1.0f + sinf(degreeToRad(roll / 2.0f)) / 2.0f) || (owner->plane->y < this->floorAltitude() && vertical_speed < 0.0f)) {
                return 10;
            }
            return 0;
        }
        case 15: {
            if (has_target || owner->pilot->autopilotActive()) {
                return 0;
            }
            if (owner->plane->wing_stall > 0 || (ctx.ias <= ctx.min_speed && owner->pilot->NosePitch() > 30.0f)) {
                return 10;
            }
            return 0;
        }
        case 16:
            return flying < 12 ? 1 : 0;
        case 19: {
            SCMissionActors *candidate = ground_target != nullptr ? ground_target : owner->target;
            bool ground_ok = candidate != nullptr && !candidate->is_destroyed && candidate->object != nullptr && candidate->object->entity != nullptr && candidate->object->entity->target_type == 2;
            return (ground_ok && (this->loadedWeaponMask() & 0xFC) != 0) ? 5 : 0;
        }
        default:
            return 0;
    }
    return (std::max)(0, (std::min)(9, score));
}

bool SCAIBrain::runTournament() {
    tournament_scores[0] = '\0';
    static const int fixed_ids[8] = {20, 14, 15, 16, 21, 7, 19, 4};
    int ids[25];
    int weights[25];
    int count = 8;
    for (int i = 0; i < 8; i++) {
        ids[i] = fixed_ids[i];
        weights[i] = 0;
    }
    for (auto &entry : owner->profile->ai.mvrs) {
        bool fixed = false;
        for (int i = 0; i < 8; i++) {
            if (ids[i] == entry.node_id) {
                weights[i] = entry.value;
                fixed = true;
            }
        }
        if (!fixed && count < 25) {
            ids[count] = entry.node_id;
            weights[count] = entry.value;
            count++;
        }
    }
    int best = -1000;
    int best_id = 0;
    int best_hint = 0;
    uint8_t best_bits = 0;
    Vector3D best_point = {0.0f, 0.0f, 0.0f};
    for (int i = 0; i < count; i++) {
        maneuver.phase_hint = 0;
        maneuver.bits = 0;
        int score = this->scoreManeuver(ids[i], air_target);
        if (score == 0) {
            continue;
        }
        int total = score + weights[i] + ((std::rand() & 1) ? 1 : -1);
        size_t used = strlen(tournament_scores);
        snprintf(tournament_scores + used, sizeof(tournament_scores) - used, "%s%d:%d", used > 0 ? " " : "", ids[i], total);
        if (total > best) {
            best = total;
            best_id = ids[i];
            best_hint = maneuver.phase_hint;
            best_bits = maneuver.bits;
            best_point = maneuver.point;
        }
    }
    if (best_id == 0) {
        return false;
    }
    maneuver.phase_hint = best_hint;
    maneuver.bits = best_bits;
    maneuver.point = best_point;
    this->applyManeuver(best_id, air_target, REACT_NONE);
    printf("AI %s#%d maneuver %d wins (score %d)\n", owner->actor_name.c_str(), owner->actor_id, best_id, best);
    return this->tickManeuver();
}

void SCAIBrain::applyManeuver(int id, SCMissionActors *target, uint8_t level) {
    maneuver.id = id;
    maneuver.target = target;
    maneuver.level = level;
    maneuver.timer = 2.0f;
    maneuver.phase = 0;
    maneuver.segments = 0;
    maneuver.start_heading = owner->plane->forward.Azimuth();
    maneuver.uses[id]++;
    maneuver.end_reason = "";
    owner->pilot->disengageAutopilot();
    this->pushBehavior(BEHAVIOR_MANEUVER);
    this->buildCombatContext(target);
    switch (id) {
        case 1: {
            float side = ctx.side >= 0.0f ? 1.0f : -1.0f;
            Vector3D level(owner->plane->forward.x, 0.0f, owner->plane->forward.z);
            level.Normalize();
            maneuver.segment = level.RotateAzimuth(30.0f * side);
            maneuver.segment_timer = 1.0f;
            maneuver.segments = 4;
            break;
        }
        case 2: {
            float stick = owner->pilot->RollStickValue();
            maneuver.side = stick > 3.0f ? 1 : stick < -3.0f ? 0 : (std::rand() & 1);
            break;
        }
        case 3:
            maneuver.timer = 4.0f;
            break;
        case 4:
            maneuver.side = ctx.side >= 0.0f ? 1 : 0;
            break;
        case 5:
            maneuver.timer = 5.0f;
            maneuver.phase = maneuver.phase_hint;
            break;
        case 6:
            maneuver.timer = 5.0f;
            maneuver.phase = maneuver.phase_hint;
            break;
        case 7:
            maneuver.timer = (maneuver.bits & 1) ? 1.0f : 2.0f;
            if ((maneuver.bits & 0x0A) != 0 && !this->tooLow()) {
                maneuver.timer = 4.0f;
            }
            maneuver.bits &= ~0x14;
            break;
        case 13:
            maneuver.timer = 4.0f;
            maneuver.phase = 1;
            break;
        case 16:
            maneuver.timer = 1.5f;
            break;
        default:
            break;
    }
}

void SCAIBrain::endManeuver(bool finished) {
    if (maneuver.id == 0) {
        return;
    }
    printf("AI %s#%d maneuver %d ends\n", owner->actor_name.c_str(), owner->actor_id, maneuver.id);
    if (maneuver.level != REACT_NONE && reaction_level == maneuver.level) {
        reaction_level = REACT_NONE;
    }
    if (maneuver.id == 19) {
        this->resetGroundAttack(finished);
    }
    last_finished_behavior = maneuver.id;
    maneuver.id = 0;
    maneuver.target = nullptr;
    maneuver.level = REACT_NONE;
    this->endBehavior(BEHAVIOR_MANEUVER, finished);
}

void SCAIBrain::maneuverSpeed(float wanted) {
    float speed = owner->plane->worldVelocity().Length();
    if (maneuver.target != nullptr && maneuver.target->plane != nullptr) {
        if (ctx.dist < 1800.0f) {
            wanted = wanted * (3600.0f - ctx.dist) / 1800.0f;
        }
        if (this->skillCheck(owner->profile->ai.atrb.FL, 0) && speed <= ctx.target_speed) {
            wanted = wanted / 2.0f;
        }
    } else {
        wanted = (float) owner->object->entity->jdyn->ai_speed_cruise;
    }
    this->speedThrottle(wanted);
}

// AI_ThrottleController_6250 : e = (vitesse voulue - ma vitesse) / 5 ; e >= 5 -> cran 10, e <= -5 -> cran -1,
// sinon cran = arrondi(e + 5)
void SCAIBrain::speedThrottle(float wanted) {
    float error = (wanted - owner->plane->worldVelocity().Length()) / 5.0f;
    float notch = error >= 5.0f ? 10.0f : error > -5.0f ? error + 5.0f : -1.0f;
    int command = (int) (notch < 0.0f ? floorf(notch - 0.5f) : floorf(notch + 0.5f));
    owner->pilot->CmdThrottle(std::clamp(command, -1, 10));
}

bool SCAIBrain::ejectDecision(int mode) {
    float current = 0.0f;
    float initial = 0.0f;
    for (auto &system : owner->object->entity->sysm) {
        auto health_system = owner->plane->system_health.find(system.first);
        for (auto &sub_system : system.second) {
            initial += (float) sub_system.second;
            if (health_system != owner->plane->system_health.end() && health_system->second.find(sub_system.first) != health_system->second.end()) {
                current += (float) health_system->second[sub_system.first];
            } else {
                current += (float) sub_system.second;
            }
        }
    }
    float damage = initial > 0.0f ? 1.0f - current / initial : 0.0f;
    bool stalled = owner->plane->wing_stall > 0;
    bool eject = damage > 0.8f || (mode == 2 && stalled) || (mode == 1 && stalled && owner->plane->y < this->floorAltitude());
    if (!eject) {
        return false;
    }
    SCMissionActors *player = this->playerActor();
    if (player != nullptr && player->team_id == owner->team_id) {
        owner->setMessage(9);
    }
    printf("AI %s#%d ejects (damage %.0f%%)\n", owner->actor_name.c_str(), owner->actor_id, damage * 100.0f);
    owner->plane->ejected = true;
    owner->plane->object->alive = false;
    return true;
}

bool SCAIBrain::tickSegment() {
    owner->pilot->CmdThrottle(10);
    owner->pilot->CmdGuidance(maneuver.segment);
    maneuver.segment_timer -= TICK_DURATION;
    return maneuver.segment_timer > 0.0f;
}

bool SCAIBrain::tickManeuver() {
    if (maneuver.id == 0) {
        return false;
    }
    SCPilot *pilot = owner->pilot;
    pilot->BeginManual();
    maneuver.timer -= TICK_DURATION;
    SCMissionActors *target = maneuver.target;
    bool has_target = target != nullptr && target->plane != nullptr && !target->is_destroyed;
    this->buildCombatContext(has_target ? target : nullptr);
    if (has_target) {
        pilot->target_waypoint = target->plane->position;
    }
    float cruise = (float) owner->object->entity->jdyn->ai_speed_cruise;
    float flying = (float) owner->profile->ai.atrb.FL;
    float climb = 40.0f * flying * flying / 256.0f + 5.0f;
    bool running = true;
    switch (maneuver.id) {
        case 1: {
            if (!has_target && maneuver.segments == 4 && maneuver.segment_timer >= 1.0f) {
                maneuver.end_reason = "sans cible";
                running = false;
                break;
            }
            if (this->tickSegment()) {
                break;
            }
            maneuver.segments--;
            Vector3D level(owner->plane->forward.x, 0.0f, owner->plane->forward.z);
            float heading_gap = 180.0f;
            if (has_target) {
                Vector3D to_target = target->plane->position - owner->plane->position;
                heading_gap = std::fabs(signed180(level.Azimuth() - to_target.Azimuth()));
            }
            if (maneuver.segments <= 0 || !has_target || heading_gap < 60.0f) {
                maneuver.end_reason = !has_target ? "sans cible" : heading_gap < 60.0f ? "cible a moins de 60 deg de cap" : "4 segments faits";
                running = false;
                break;
            }
            level.Normalize();
            maneuver.segment = level.RotateAzimuth(-60.0f);
            maneuver.segment_timer = 1.0f;
            break;
        }
        case 2: {
            if (maneuver.timer < 0.0f) {
                maneuver.end_reason = "minuteur";
                running = false;
                break;
            }
            if (!has_target) {
                maneuver.end_reason = "sans cible";
                running = false;
                break;
            }
            Vector3D delta = -owner->plane->forward;
            Matrix &m = owner->plane->ptw;
            float up[3] = {m.v[1][0], m.v[1][1], m.v[1][2]};
            float d[3] = {delta.x, delta.y, delta.z};
            int axis = 0;
            for (int i = 1; i < 3; i++) {
                if (std::fabs(d[i]) > std::fabs(d[axis])) {
                    axis = i;
                }
            }
            bool same = (up[axis] >= 0.0f) == (d[axis] >= 0.0f);
            pilot->CmdPitchStick(same ? 16.0f : -8.0f);
            pilot->CmdRollStick(maneuver.side == 1 ? 16.0f : -16.0f);
            this->maneuverSpeed(ctx.target_speed);
            break;
        }
        case 3:
            if (maneuver.phase == 1 && maneuver.segment_timer > 0.0f) {
                maneuver.timer += TICK_DURATION;
            } else if (maneuver.timer < 0.0f && maneuver.phase != 6) {
                maneuver.phase = 6;
            }
            switch (maneuver.phase) {
                case 0: {
                    bool slow = this->tooSlow();
                    bool low = this->tooLow();
                    if (slow && !low) {
                        maneuver.phase = 2;
                    } else if (slow && low) {
                        maneuver.phase = 1;
                    } else if (!slow && low) {
                        maneuver.phase = 4;
                    } else {
                        int pick = std::rand() & 3;
                        maneuver.phase = pick == 1 ? 2 : pick == 2 ? 4 : 1;
                    }
                    maneuver.segment_timer = 0.0f;
                    break;
                }
                case 1: {
                    if (maneuver.segment_timer <= 0.0f) {
                        float magnitude = (std::rand() & 0x20) != 0 ? 64.0f : 32.0f;
                        if (maneuver.jink_flip) {
                            magnitude = -magnitude;
                        }
                        maneuver.jink_flip = !maneuver.jink_flip;
                        Vector3D level(owner->plane->forward.x, 0.0f, owner->plane->forward.z);
                        level.Normalize();
                        maneuver.segment = level.RotateAzimuth(magnitude);
                        maneuver.segment_timer = 1.0f;
                        maneuver.timer -= 1.0f;
                    }
                    if (!this->tickSegment()) {
                        maneuver.segment_timer = 0.0f;
                        maneuver.phase = 0;
                    }
                    break;
                }
                case 2:
                    this->speedThrottle(cruise);
                    if (pilot->NosePitch() >= -30.0f) {
                        maneuver.phase = 3;
                    }
                    if (pilot->CmdRollTo(180.0f, 15.0f)) {
                        pilot->CmdPitchStick(16.0f);
                    }
                    break;
                case 3:
                    if (this->tooLow() || ctx.ias >= cruise) {
                        if (pilot->CmdPitchTo(5.0f, 10.0f)) {
                            maneuver.phase = 0;
                        }
                    } else {
                        pilot->CmdPitchTo(-climb, 10.0f);
                    }
                    this->speedThrottle(cruise);
                    break;
                case 4:
                    this->speedThrottle(cruise);
                    if (pilot->NosePitch() <= 30.0f) {
                        maneuver.phase = 5;
                    }
                    if (pilot->CmdRollTo(0.0f, 15.0f)) {
                        pilot->CmdPitchStick(16.0f);
                    }
                    break;
                case 5:
                    if (owner->plane->y > this->floorAltitude() + 3.0f * 200.0f || this->tooSlow()) {
                        if (pilot->CmdPitchTo(-5.0f, 10.0f)) {
                            maneuver.phase = 0;
                        }
                    } else {
                        pilot->CmdPitchTo(climb, 10.0f);
                    }
                    this->speedThrottle(cruise);
                    break;
                default:
                    this->speedThrottle(cruise);
                    if (pilot->CmdPitchTo(0.0f, 5.0f)) {
                        maneuver.end_reason = "minuteur puis remise a plat";
                        running = false;
                    }
                    break;
            }
            break;
        case 4: {
            float turned = std::fabs(signed180(owner->plane->forward.Azimuth() - maneuver.start_heading));
            if (turned >= 90.0f || maneuver.timer < 0.0f) {
                if (pilot->CmdPitchTo(10.0f, 10.0f)) {
                    maneuver.end_reason = turned >= 90.0f ? "cap tourne de 90 deg puis assiette +10" : "minuteur puis assiette +10";
                    running = false;
                } else if (maneuver.timer < -2.0f) {
                    maneuver.end_reason = "minuteur depasse de 2 s";
                    running = false;
                }
                break;
            }
            float bank = this->tooLow() ? 60.0f : 90.0f;
            float wanted = maneuver.side == 1 ? bank : -bank;
            if (pilot->CmdRollTo(wanted, 30.0f)) {
                pilot->CmdRollTo(wanted, 5.0f);
                pilot->CmdPitchStick(16.0f);
            }
            break;
        }
        case 5:
        case 6: {
            if (maneuver.timer <= 0.0f) {
                maneuver.end_reason = "minuteur";
                running = false;
                break;
            }
            // MVRS_ID5_TickVerticalReversal_1011F (loc_1025E) : vitesse max JDYN+0x80 en phases 2 a 4
            if (maneuver.id == 5 && maneuver.phase >= 2 && maneuver.phase <= 4) {
                this->speedThrottle((float) owner->object->entity->jdyn->ai_speed_max);
            } else {
                this->speedThrottle(cruise);
            }
            Vector3D direction = has_target ? ctx.D : owner->plane->forward;
            int phase_before = maneuver.phase;
            switch (maneuver.phase) {
                case 1:
                    if (maneuver.id == 5) {
                        if (owner->plane->y > (float) owner->mission->intel.unknown_range) {
                            pilot->CmdPitchTo(-30.0f, 5.0f);
                        } else {
                            maneuver.phase = 2;
                        }
                    } else {
                        if (pilot->CmdPitchTo(30.0f, 5.0f) && owner->plane->y > this->floorAltitude() + 2000.0f) {
                            maneuver.phase = 2;
                        } else if (owner->plane->y > this->floorAltitude() + 2000.0f) {
                            maneuver.phase = 2;
                        }
                    }
                    break;
                case 2:
                    if (maneuver.id == 5) {
                        pilot->CmdPitchTo(this->tooLow() ? 5.0f : -30.0f, 5.0f);
                        if (owner->plane->worldVelocity().Length() >= cruise) {
                            maneuver.phase = 3;
                        }
                    } else {
                        this->speedThrottle((float) owner->object->entity->jdyn->ai_speed_min);
                        pilot->CmdPitchTo(20.0f, 5.0f);
                        if (owner->plane->worldVelocity().Length() < cruise) {
                            maneuver.phase = 3;
                        }
                    }
                    break;
                case 3:
                    if (maneuver.id == 5) {
                        if (pilot->CmdPitchTo(0.0f, 5.0f) && pilot->CmdRollTo(0.0f, 5.0f)) {
                            maneuver.phase = 4;
                        }
                    } else if (pilot->CmdPitchTo(0.0f, 5.0f) && pilot->CmdRollTo(180.0f, 5.0f)) {
                        maneuver.phase = 4;
                    }
                    break;
                case 4:
                    if (pilot->CmdPitchTo(maneuver.id == 5 ? 90.0f : -90.0f, 5.0f)) {
                        maneuver.phase = 5;
                    }
                    break;
                case 5: {
                    float r = pilot->BearingToRef(direction);
                    pilot->CmdRollTo(pilot->BankAngle() + r, 5.0f);
                    if (std::fabs(r) < 5.0f || this->tooSlow()) {
                        maneuver.phase = 6;
                    }
                    break;
                }
                case 6:
                    pilot->CmdPitchStick(16.0f);
                    if (maneuver.id == 5 ? pilot->NosePitch() <= 45.0f : pilot->NosePitch() >= -45.0f) {
                        maneuver.phase = 7;
                    }
                    break;
                case 7:
                    if (pilot->CmdPitchTo(has_target ? direction.Elevation() : 0.0f, 5.0f)) {
                        maneuver.phase = 8;
                    }
                    break;
                default:
                    if (pilot->CmdRollTo(0.0f, 5.0f)) {
                        maneuver.end_reason = "ailes a plat (phase 8)";
                        running = false;
                    }
                    break;
            }
            // loc_10AA1 / loc_105B9 : minuteur remis a 10 s a chaque changement de phase
            if (maneuver.phase != phase_before) {
                maneuver.timer = 10.0f;
            }
            break;
        }
        case 7: {
            // MVRS_ID7_TickPursuit_10BD9
            if (!has_target) {
                maneuver.end_reason = "sans cible";
                running = false;
                break;
            }
            // minuteur ecoule : Behavior_PopFinished_75612, le tick va quand meme au bout
            bool expired = maneuver.timer < 0.0f;
            if (maneuver.bits & 2) {
                maneuver.pursuit_aspect = ctx.target_vel_angle;
            }
            if ((maneuver.bits & 4) || ((maneuver.bits & 2) && maneuver.pursuit_aspect >= 80.0f)) {
                if (maneuver.bits & 2) {
                    maneuver.bits = (maneuver.bits & ~2) | 4;
                } else if (maneuver.pursuit_aspect < 80.0f) {
                    maneuver.bits &= ~4;
                }
                // point decale memorise (horizontal), montee a 45 deg si pas trop lent
                Vector3D direction(maneuver.point.x, 0.0f, maneuver.point.z);
                if (!this->tooSlow()) {
                    direction.y = sqrtf(direction.x * direction.x + direction.z * direction.z);
                }
                pilot->CmdGuidance(direction);
                this->speedThrottle(this->referenceSpeed());
            } else if (maneuver.bits & 0x10) {
                if (ctx.nose_angle <= 45.0f) {
                    this->interceptDispatcher(target);
                    maneuver.bits &= ~0x10;
                } else if (pilot->CmdGuidance(maneuver.point)) {
                    // l'original passe la position absolue memorisee comme direction (AI_GuidanceCmd_FromOwnPos)
                    maneuver.bits &= ~0x10;
                }
            } else if (this->collisionCourse(target)) {
                // AI_CollisionCourseTest_DD21 : seul le tick s'arrete (jmp loc_11159), la poursuite continue
            } else if (maneuver.bits & 8) {
                float threshold = 2.0f * ctx.target_speed * ctx.target_speed / 9.0f;
                if (ctx.dist <= threshold) {
                    maneuver.timer = 4.0f;
                    maneuver.bits = (maneuver.bits | 0x10) & ~8;
                    maneuver.point = target->plane->position + target->plane->worldVelocity() * 4.0f;
                } else {
                    this->interceptDispatcher(target);
                }
            } else {
                if (ctx.nose_angle <= 60.0f && ctx.aspect < 60.0f) {
                    maneuver.bits |= 8;
                    this->interceptDispatcher(target);
                } else if (this->interceptDispatcher(target)) {
                    maneuver.end_reason = "aligne ou recalage de visee";
                    running = false;
                }
            }
            if (expired) {
                maneuver.end_reason = "minuteur";
                running = false;
            }
            break;
        }
        case 13: {
            if (!has_target) {
                maneuver.end_reason = "sans cible";
                running = false;
                break;
            }
            float horizontal = sqrtf(ctx.D.x * ctx.D.x + ctx.D.z * ctx.D.z);
            // MVRS_ID13_TickZoomClimb_113FD
            if (maneuver.phase == 1) {
                if (horizontal < (float) owner->mission->intel.range_long) {
                    maneuver.end_reason = "cible a moins de 17 700 m";
                    running = false;
                    break;
                }
                // AI_GuidanceCmd_FromOwnPos(D horizontal, 0Ah) : le 0Ah n'est pas un cran de manette
                Vector3D direction = ctx.D;
                direction.y = 0.0f;
                if (pilot->CmdGuidance(direction)) {
                    maneuver.phase = 2;
                }
            } else if (maneuver.phase == 2) {
                if (this->tooSlow()) {
                    if (pilot->CmdPitchTo(0.0f, 5.0f)) {
                        pilot->CmdRollTo(0.0f, 5.0f);
                    }
                } else {
                    maneuver.phase = 3;
                }
                pilot->CmdThrottle(10);
            } else if (maneuver.phase == 3) {
                // sortie testee avant le second decompte du minuteur, sur la distance 3D
                if (maneuver.timer < 0.0f || this->tooSlow() || ctx.dist < (float) owner->mission->intel.range_long) {
                    maneuver.phase = 4;
                }
                maneuver.timer -= TICK_DURATION;
                float angle = 30.0f;
                if (ctx.ias > cruise) {
                    float min_speed = (float) owner->object->entity->jdyn->ai_speed_min;
                    angle = (std::min)(60.0f, 30.0f + 30.0f * (ctx.ias - cruise) / min_speed);
                }
                if (pilot->CmdPitchTo(angle, 5.0f)) {
                    pilot->CmdRollTo(0.0f, 5.0f);
                }
                pilot->CmdThrottle(10);
            } else {
                this->speedThrottle(cruise);
                if (pilot->CmdPitchTo(0.0f, 5.0f)) {
                    maneuver.end_reason = "remise a plat (phase 4)";
                    running = false;
                }
            }
            break;
        }
        case 14: {
            bool eject = this->ejectDecision(2);
            if (eject || maneuver.timer < 0.0f) {
                maneuver.end_reason = eject ? "ejection" : "minuteur";
                running = false;
                break;
            }
            float vertical_speed = owner->plane->worldVelocity().y;
            if (vertical_speed > 0.0f && owner->plane->y > this->floorAltitude()) {
                pilot->CmdPitchTo(30.0f, 10.0f);
                pilot->CmdThrottle(10);
                maneuver.end_reason = "en montee au-dessus du plancher";
                running = false;
                break;
            }
            bool nose_down = pilot->NosePitch() < 0.0f;
            pilot->CmdThrottle(nose_down && ctx.ias > this->referenceSpeed() ? 1 : 10);
            pilot->CmdPitchTo(30.0f, 10.0f);
            break;
        }
        case 15: {
            if (maneuver.timer < 0.0f) {
                maneuver.end_reason = "minuteur";
                running = false;
                break;
            }
            if (this->ejectDecision(1)) {
                maneuver.end_reason = "ejection";
                running = false;
                break;
            }
            bool stalled = owner->plane->wing_stall > 0;
            if ((!stalled || ctx.ias > this->referenceSpeed()) && pilot->NosePitch() <= 0.0f) {
                maneuver.end_reason = "nez sous l'horizon";
                running = false;
                break;
            }
            if (pilot->NosePitch() > 60.0f && stalled) {
                pilot->CmdThrottle(0);
            } else {
                pilot->CmdThrottle(10);
                if (!stalled) {
                    pilot->CmdPitchTo(-30.0f, 10.0f);
                }
            }
            break;
        }
        case 16: {
            if (maneuver.timer < 0.0f || ctx.ias >= (cruise + this->referenceSpeed()) / 2.0f) {
                maneuver.end_reason = maneuver.timer < 0.0f ? "minuteur" : "vitesse atteinte";
                running = false;
                break;
            }
            this->speedThrottle((float) owner->object->entity->jdyn->ai_speed_max);
            pilot->CmdPitchTo(5.0f, 5.0f);
            break;
        }
        case 19:
            if (ground_target == nullptr) {
                maneuver.end_reason = "sans cible au sol";
                running = false;
                break;
            }
            pilot->ClearGuidance();
            this->updateGroundAttack(ground_target);
            running = ground_attack_active;
            if (!running) {
                maneuver.end_reason = "attaque au sol terminee";
            }
            break;
        case 20:
            // MVRS_ID20_TickApplyGuidance_11A04 : direction memorisee, plein gaz, pendant le minuteur
            running = this->tickSegment();
            if (!running) {
                maneuver.end_reason = "segment fini";
            }
            break;
        default:
            maneuver.end_reason = "manoeuvre sans tick";
            running = false;
            break;
    }
    if (!running) {
        this->endManeuver(true);
        owner->pilot->ClearGuidance();
    } else if (debug_ticks % 25 == 0) {
        printf("AI %s#%d maneuver %d phase=%d timer=%.1f bank=%.0f pitch=%.0f\n", owner->actor_name.c_str(), owner->actor_id, maneuver.id, maneuver.phase, maneuver.timer, pilot->BankAngle(), pilot->NosePitch());
    }
    return running;
}

bool SCAIBrain::runReflexes() {
    if (maneuver.id != 0 || owner->plane->on_ground || owner->pilot->autopilotActive()) {
        return false;
    }
    if (reaction_level <= REACT_STALL_RECOVERY && this->scoreManeuver(15, nullptr) > 0) {
        this->applyManeuver(15, nullptr, REACT_STALL_RECOVERY);
        reaction_level = REACT_STALL_RECOVERY;
        printf("AI %s#%d stall recovery reflex\n", owner->actor_name.c_str(), owner->actor_id);
        this->tickManeuver();
        return true;
    }
    if (reaction_level <= REACT_GROUND_AVOID && this->scoreManeuver(14, nullptr) > 0) {
        this->applyManeuver(14, nullptr, REACT_GROUND_AVOID);
        reaction_level = REACT_GROUND_AVOID;
        printf("AI %s#%d ground avoid reflex\n", owner->actor_name.c_str(), owner->actor_id);
        this->tickManeuver();
        return true;
    }
    return false;
}

// AI_ScanCollisionThreats_DF99 : evitement de collision avec tous les avions, amis compris (niveau 3)
bool SCAIBrain::scanCollisionThreats() {
    if (this->behaviorRunning()) {
        if (reaction_level == REACT_COLLISION) {
            this->tickBehavior();
            return true;
        }
        return false;
    }
    if (reaction_level == REACT_COLLISION) {
        reaction_level = REACT_NONE;
    }
    for (auto actor : owner->mission->actors) {
        if (actor == owner || actor->plane == nullptr || actor->is_destroyed || actor->object->entity->target_type != 1) {
            continue;
        }
        if (this->collisionCourse(actor)) {
            return true;
        }
    }
    return false;
}

// AI_CollisionCourseTest_DD21 : a moins de 80 m, ou passage a moins de 80 m dans les 4 s
bool SCAIBrain::collisionCourse(SCMissionActors *other) {
    Vector3D away = owner->plane->position - other->plane->position;
    float distance = away.Length();
    Vector3D other_velocity = other->plane->worldVelocity();
    Vector3D closing = other_velocity - owner->plane->worldVelocity();
    float closing_speed = closing.Length();
    bool collision = distance < 80.0f;
    if (!collision && closing_speed > 0.0f) {
        float angle = closing.AngleBetween(away);
        if (angle < 90.0f) {
            float miss = distance * sinf(degreeToRad(angle));
            float along = distance * cosf(degreeToRad(angle));
            collision = miss < 80.0f && along <= 4.0f * closing_speed;
        }
    }
    if (!collision) {
        return false;
    }
    // AI_ProximityGeometricWarning_315B : ecart perpendiculaire a la visee et a sa trajectoire a 4 s,
    // score ID20 nul (< 5) -> vecteur inverse ; quasi nul -> Utility_Helper_55DB4 (5, 5, 1000) en axes asm
    Vector3D side = -Vector3D::Cross(away, other_velocity * 4.0f);
    if (side.Length() < 1.0f) {
        side = Vector3D(5.0f, 1000.0f, -5.0f);
    }
    this->abandonBehavior();
    this->applyManeuver(20, nullptr, REACT_COLLISION);
    maneuver.segment = side;
    maneuver.segment_timer = 2.0f;
    reaction_level = REACT_COLLISION;
    printf("AI %s#%d collision avoidance with %s d=%.0f\n", owner->actor_name.c_str(), owner->actor_id, other->actor_name.c_str(), distance);
    this->tickManeuver();
    return true;
}

void SCAIBrain::incomingThreatWarning() {
    if (owner->plane->on_ground || reaction_level > REACT_ENGAGED || maneuver.id == 4 || threat_alerted) {
        if (maneuver.id != 4) {
            threat_alerted = false;
        }
        return;
    }
    if (owner->profile->ai.atrb.FL < 13) {
        return;
    }
    for (auto actor : owner->mission->actors) {
        if (actor->team_id == owner->team_id || actor->plane == nullptr || actor->is_destroyed || actor->brain == nullptr || actor->brain->air_target != owner) {
            continue;
        }
        Vector3D delta = actor->plane->position - owner->plane->position;
        if (delta.Length() > owner->mission->intel.range_gun) {
            continue;
        }
        Vector3D closing = actor->plane->worldVelocity() - owner->plane->worldVelocity();
        if (closing.x * delta.x + closing.y * delta.y + closing.z * delta.z >= 0.0f) {
            continue;
        }
        if (!this->skillCheck(owner->profile->ai.atrb.FL, 0)) {
            return;
        }
        this->endManeuver(false);
        this->applyManeuver(4, actor, REACT_NONE);
        this->decide("alerte de menace : ID 4 contre %s", actor->actor_name.c_str());
        threat_alerted = true;
        printf("AI %s#%d threat warning from %s: defensive turn\n", owner->actor_name.c_str(), owner->actor_id, actor->actor_name.c_str());
        return;
    }
}
