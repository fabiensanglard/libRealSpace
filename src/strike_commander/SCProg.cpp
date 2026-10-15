#include "precomp.h"
#include "../engine/gametimer.h"


/**
 * @brief Interpret and execute the scripted program (bytecode) attached to this SCProg instance.
 *
 * This method walks sequentially through the vector of PROG instructions (this->prog) and
 * interprets each opcode, mutating actor state, mission state, flags, command queues, and
 * various runtime control variables used for mission logic (e.g. branching, sub‑program
 * invocation, conditional evaluation).
 *
 * Control Flow Mechanism:
 * - A simple linear interpreter with manual flow control using:
 *   - jump_to + OP_SET_LABEL + setting exec=false to defer execution until a matching label.
 *   - Conditional gotos (OP_GOTO_LABEL_IF_TRUE / OP_GOTO_LABEL_IF_FALSE /
 *     OP_IF_LESS_THAN_GOTO / OP_IF_GREATER_THAN_GOTO).
 *   - Sub‑program execution via OP_EXEC_SUB_PROG (recursively creates and runs a new SCProg).
 *   - Indirect jump via OP_EXECUTE_CALL (jump target taken from work_register).
 * - The variable 'exec' gates whether the current instruction is active. When a jump is
 *   scheduled, exec is set to false until a matching OP_SET_LABEL with the desired label id
 *   (jump_to) is encountered, at which point execution resumes.
 *
 * State / Working Registers:
 * - work_register: General purpose integer scratch register used by arithmetic, comparisons,
 *   indirect jumps, and data movement (e.g. flags <-> work register).
 * - true_flag / compare_flag: Results of condition evaluation (boolean or tri-state compare)
 *   drive conditional jumps and branching logic.
 * - jump_to: Holds the label id to resume execution at after a jump is requested.
 * - call_to (currently unused) and flag_number (currently unused) appear reserved for future
 *   extensions.
 *
 * Side Effects on Actor:
 * - Clears and repopulates actor->executed_opcodes with indices of instructions actually
 *   executed (some conditional instructions only push if taken).
 * - Issues or updates current command (take off, land, move, destroy target, defend, follow,
 *   etc.) and tracks its completion state (current_command_executed).
 * - Queries spatial / combat related conditions (distance to target / spot, target alive,
 *   target in area) to set true_flag.
 * - Can instantly destroy a target actor (OP_INSTANT_DESTROY_TARGET), optionally spawning
 *   explosion effects.
 * - Can activate/deactivate actors or scenes.
 * - Sets in‑game messages via OP_SET_MESSAGE.
 *
 * Side Effects on Mission:
 * - Reads / writes mission->mission->mission_data.flags (increment, decrement, set boolean,
 *   arithmetic with work_register).
 * - Stores to mission->gameflow_registers via OP_SAVE_VALUE_TO_GAMFLOW_REGISTER.
 * - Activates scenes matching an area id.
 *
 * Comparison & Branching:
 * - OP_CMP_GREATER_EQUAL_THAN sets compare_flag to -1 / 0 / 1 for less / equal / greater,
 *   and sets true_flag to true if >=, false otherwise.
 * - Subsequent conditional opcodes test compare_flag or true_flag to decide jumps.
 *
 * Sub‑program Execution:
 * - OP_EXEC_SUB_PROG looks up a referenced PROG vector in mission data, constructs a temporary
 *   SCProg, and executes it immediately (recursive interpretation).
 *
 * Notable Implementation Details / Caveats:
 * - Some fields (call_to, flag_number) are declared but never used.
 * - true_flag is sometimes reset implicitly (e.g., only changed by certain opcodes); logic that
 *   depends on stale values must ensure an opcode establishing it has run.
 * - No bounds checking for many flag / actor indices (assumes valid script).
 * - No cycle detection; malicious or ill‑formed scripts with self‑referential jumps could stall
 *   progression (although loop constructs rely on label scanning rather than rewinding i).
 *
 * Logging / Tracing:
 * - The order and subset of executed instructions can be reconstructed from actor->executed_opcodes,
 *   which stores the numeric index (i) of each opcode actually processed (including some that
 *   only trigger under conditional execution).
 *
 * Thread Safety:
 * - Not thread‑safe; mutates shared mission and actor state without synchronization.
 *
 * Performance:
 * - Single pass over instructions; label resolution is linear (no precomputed label index), so
 *   large scripts with many jumps could incur extra scanning cost.
 *
 * Lifecycle:
 * - Returns immediately upon first OP_EXIT_PROG unless true_flag was set (in which case it clears
 *   true_flag and continues), enabling a "conditional early exit" pattern.
 *
 * Preconditions:
 * - this->actor, this->mission, and mission->mission->mission_data structures must be valid.
 * - Flags / actors / scenes referenced by indices must exist.
 *
 * Postconditions:
 * - Actor command state, flags, and mission side effects reflect all executed opcodes up to
 *   normal termination or early exit.
 *
 *
 * @return void (effects are applied through side effects).
 */
void SCProg::execute() {
    uint8_t i = 0;
    size_t jump_to = 0;
    size_t call_to = 0;
    int flag_number = 0;
    int work_register = 0;
    int compare_flag = prog_compare_return_values::PROG_CMP_UNSET;
    bool exec = true;
    bool objective_flag = false;
    bool true_flag = false;
    SPOT *spot = nullptr;
    this->actor->executed_opcodes.clear();
    this->actor->executed_opcodes.shrink_to_fit();
    this->mission->progs_traces[this->prog_id].clear();
    this->mission->progs_traces[this->prog_id].shrink_to_fit();
    for (auto prog : this->prog) {
        if (exec || (!exec && jump_to == prog.arg && prog.opcode == OP_SET_LABEL)) {
            this->actor->executed_opcodes.push_back(i);
            this->mission->progs_traces[this->prog_id].push_back(i);
            switch (prog.opcode) {
                case OP_EXIT_PROG:
                    return;
                break;
                case OP_SPOT_DATA:
                    if (prog.arg < this->mission->mission->mission_data.spots.size()) {
                        spot = this->mission->mission->mission_data.spots[prog.arg];
                    }
                break;
                case OP_SET_LABEL:
                    if (jump_to == prog.arg) {
                        exec = true;
                    }
                break;
                case OP_MOVE_VALUE_TO_WORK_REGISTER:
                    work_register = prog.arg;
                break;
                case OP_GOTO_IF_CURRENT_COMMAND_IN_PROGRESS:
                    if (this->task_state != 0) {
                        jump_to = prog.arg;
                        exec = false;
                    }
                break;
                case OP_IF_TARGET_IN_AREA:
                    if (this->actor->ifTargetInSameArea(prog.arg)) {
                        compare_flag = prog_compare_return_values::PROG_CMP_EQUAL;
                    } else {
                        compare_flag = prog_compare_return_values::PROG_CMP_NOT_EQUAL;
                    }
                break;
                case OP_ADD_1_TO_FLAG:
                    this->mission->mission->mission_data.flags[prog.arg]++;
                break;
                case OP_REMOVE_1_TO_FLAG:
                    this->mission->mission->mission_data.flags[prog.arg]--;
                break;
                case OP_ADD_WORK_REGISTER_TO_FLAG:
                    this->mission->mission->mission_data.flags[prog.arg] += work_register;
                break;
                case OP_INSTANT_DESTROY_TARGET:
                    for (auto actor: this->mission->actors) {
                        if (actor->actor_id == prog.arg) {
                            actor->object->alive = false;
                            actor->is_destroyed = false;
                            if (actor->object->entity->explos != nullptr && actor->object->entity->explos->objct != nullptr) {
                                actor->mission->explosions.push_back(new SCExplosion(actor->object->entity->explos->objct, actor->object->position));
                            }
                            break;
                        }
                    }
                break;
                case OP_SET_WAIT_FOR_SECONDS: {
                    // Expr_VM_Interpreter_51106 cas 0xA0 : minuteur de l'objet de mission (+0x3A), global pour une scene
                    float &timer = this->scene_script ? this->mission->scene_wait_timer : this->actor->wait_timer;
                    this->task_state = 1;
                    if (timer == 0.0f) {
                        timer = (float) prog.arg;
                    } else {
                        timer -= GameTimer::getInstance().getDeltaTime();
                        if (timer <= 0.0f) {
                            timer = 0.0f;
                            this->task_state = 0;
                        }
                    }
                }
                break;
                case OP_SET_OBJ_TAKE_OFF:
                    this->task_state = this->actor->setObjective(OP_SET_OBJ_TAKE_OFF, prog.arg) ? 1 : 0;
                break;
                case OP_SET_OBJ_LAND:
                    this->task_state = this->actor->setObjective(OP_SET_OBJ_LAND, prog.arg) ? 1 : 0;
                    if (this->actor->current_command == OP_SET_OBJ_LAND && this->actor->current_command_arg == prog.arg) {
                        // Expr_VM_ReadNextToken_50F85 : un opcode 9 qui suit fournit le 2e operande (spot de toucher)
                        bool has_second = (size_t) i + 1 < this->prog.size() && this->prog[i + 1].opcode == OP_SPOT_DATA;
                        this->actor->current_command_arg2 = has_second ? (uint8_t) this->prog[i + 1].arg : 0xFF;
                    }
                break;
                case OP_SET_OBJ_FLY_TO_WP:
                    this->task_state = this->actor->setObjective(OP_SET_OBJ_FLY_TO_WP, prog.arg) ? 1 : 0;
                    if (this->actor->current_command == OP_SET_OBJ_FLY_TO_WP && this->actor->current_command_arg == prog.arg) {
                        // 2e operande (opcode 9) : vitesse voulue a l'arrivee (Goal_SetObjective_A307 -> +0x12B)
                        bool has_second = (size_t) i + 1 < this->prog.size() && this->prog[i + 1].opcode == OP_SPOT_DATA;
                        this->actor->current_command_arg2 = has_second ? (uint8_t) this->prog[i + 1].arg : 0xFF;
                    }
                break;
                case OP_SET_OBJ_FLY_TO_AREA:
                    // Expr_VM_Interpreter_51106 : 0xA6 -> cas par defaut, pas d'appel natif ; seul le joueur l'exploite
                    this->actor->setObjective(OP_SET_OBJ_FLY_TO_AREA, prog.arg);
                break;
                case OP_SET_OBJ_DESTROY_TARGET:
                    this->task_state = this->actor->setObjective(OP_SET_OBJ_DESTROY_TARGET, prog.arg) ? 1 : 0;
                break;
                case OP_SET_OBJ_DEFEND_TARGET:
                    this->task_state = this->actor->setObjective(OP_SET_OBJ_DEFEND_TARGET, prog.arg) ? 1 : 0;
                break;
                case OP_SET_OBJ_DEFEND_AREA:
                    this->task_state = this->actor->setObjective(OP_SET_OBJ_DEFEND_AREA, prog.arg) ? 1 : 0;
                break;
                case OP_SET_MESSAGE:
                    this->actor->setMessage(prog.arg);
                break;
                case OP_SET_OBJ_FOLLOW_ALLY: {
                    // MissionScript_CallNativeHandler_52513 cas 0xAA : 0xFF = allie par defaut (PART octet 46),
                    // poste = mots PART 48/50/52 (cote, avant, haut)
                    uint8_t leader_arg = (uint8_t) prog.arg;
                    MISN_PART *part = this->actor->object;
                    if (leader_arg == 0xFF && part != nullptr) {
                        leader_arg = part->unknown_bytes[3];
                    }
                    this->task_state = this->actor->setObjective(OP_SET_OBJ_FOLLOW_ALLY, leader_arg) ? 1 : 0;
                    if (part != nullptr && this->actor->current_command == OP_SET_OBJ_FOLLOW_ALLY && this->actor->current_command_arg == leader_arg) {
                        std::vector<uint8_t> &b = part->unknown_bytes;
                        this->actor->follow_slot = Vector3D((float) (int16_t) (b[5] | (b[6] << 8)), (float) (int16_t) (b[7] | (b[8] << 8)), (float) (int16_t) (b[9] | (b[10] << 8)));
                        this->actor->follow_slot_set = true;
                    }
                }
                break;
                case OP_DEACTIVATE_OBJ:
                    // Expr_VM_Interpreter_51106 cas 0x91 : Expr_Node_ClearDirtyAndNotify_5242A (inverse de 0x90)
                    this->actor->deactivate(prog.arg);
                break;
                case OP_SHOW_ON_NAV_MAP:
                case OP_HIDE_FROM_NAV_MAP: {
                    // Expr_VM_Interpreter_51106 cas 0xB6 / 0xB7 : bit 4 de +0x39, lu par NavMap_DrawMissionObjects_7EDCA ; 255 = soi
                    SCMissionActors *shown = nullptr;
                    if (prog.arg == 255) {
                        shown = this->actor;
                    } else {
                        for (auto candidate : this->mission->actors) {
                            if (candidate->actor_id == prog.arg) {
                                shown = candidate;
                                break;
                            }
                        }
                    }
                    if (shown != nullptr) {
                        shown->on_nav_map = prog.opcode == OP_SHOW_ON_NAV_MAP;
                    }
                }
                break;
                case OP_JOIN_PLAYER_TEAM:
                case OP_LEAVE_PLAYER_TEAM: {
                    // Expr_VM_Interpreter_51106 cas 0xB8 / 0xB9 : bit 5 de +0x39 (suit le joueur au pilote automatique,
                    // lu par MissionScenario_QueryFieldWrapper_A7FF0 depuis UIScript_ParseAndEvaluate_7A054) ; 255 = soi
                    SCMissionActors *member = nullptr;
                    if (prog.arg == 255) {
                        member = this->actor;
                    } else {
                        for (auto candidate : this->mission->actors) {
                            if (candidate->actor_id == prog.arg) {
                                member = candidate;
                                break;
                            }
                        }
                    }
                    if (member != nullptr) {
                        std::vector<SCMissionActors *> &team = this->mission->friendlies;
                        auto it = std::find(team.begin(), team.end(), member);
                        if (prog.opcode == OP_JOIN_PLAYER_TEAM && it == team.end()) {
                            team.push_back(member);
                        } else if (prog.opcode == OP_LEAVE_PLAYER_TEAM && it != team.end()) {
                            team.erase(it);
                        }
                    }
                }
                break;
                case OP_PACIFY_TARGET:
                    {
                        for (auto actr: this->actor->mission->actors) {
                            if (actr->actor_id == prog.arg && actr->is_active == true && actr->is_destroyed == false) {
                                if (actr->plane != nullptr) {
                                    bool task_result = actr->setObjective(OP_SET_OBJ_PATROL_POSITION, prog.arg);
                                    if (task_result) {
                                        this->task_state = 1;
                                    } else {
                                        this->task_state = 0;
                                    }
                                } else {
                                    if (actr->object->entity->entity_type == EntityType::swpn && actr->object->entity->swpn_data != nullptr) {
                                        actr->object->entity->swpn_data->weapons_round = 0;
                                    }
                                }
                            }
                        }
                    }
                    break;
                case OP_SET_OBJ_PATROL_POSITION:
                    this->actor->setObjective(OP_SET_OBJ_PATROL_POSITION, prog.arg);
                break;
                case OP_ACTIVATE_OBJ:
                    this->actor->activateTarget(prog.arg);
                break;
                case OP_MOVE_FLAG_TO_WORK_REGISTER:
                    work_register = this->mission->mission->mission_data.flags[prog.arg];
                break;
                case OP_SAVE_VALUE_TO_GAMFLOW_REGISTER:
                    this->mission->gameflow_registers[prog.arg] = work_register;
                break;
                case OP_MOVE_WORK_REGISTER_TO_FLAG:
                    this->mission->mission->mission_data.flags[prog.arg] = (uint8_t) work_register;
                break;
                case OP_EXECUTE_CALL:
                    jump_to = work_register;
                    exec = false;
                break;
                case OP_EXEC_SUB_PROG:
                {
                    if (prog.arg < this->mission->mission->mission_data.prog.size()) {
                        std::vector<PROG> *sub_prog = this->mission->mission->mission_data.prog[prog.arg];
                        SCProg *sub_prog_obj = new SCProg(this->actor, *sub_prog, this->mission, prog.arg);
                        sub_prog_obj->task_state = this->task_state;
                        sub_prog_obj->scene_script = this->scene_script;
                        sub_prog_obj->execute();
                        this->task_state = sub_prog_obj->task_state;
                        delete sub_prog_obj;
                    } else {
                        // Invalid sub-program index; no operation performed.
                    }
                }
                break;
                case OP_CMP_WORK_WITH_VALUE:
                {
                    compare_flag = prog_compare_return_values::PROG_CMP_UNSET;
                    bool is_equal = (work_register == prog.arg);
                    bool is_less = (work_register < prog.arg);
                    bool is_greater = (work_register > prog.arg);
                    
                    if (is_equal) {
                        compare_flag |= PROG_CMP_EQUAL;
                    }
                    if (is_less) {
                        compare_flag |= PROG_CMP_LESS;
                    }
                    if (is_greater) {
                        compare_flag |= PROG_CMP_GREATER;
                    }
                }
                break;
                case OP_CMP_VALUE_WITH_WORK:
                {
                    compare_flag = prog_compare_return_values::PROG_CMP_UNSET;
                    bool is_equal = (prog.arg == work_register);
                    bool is_less = (prog.arg < work_register);
                    bool is_greater = (prog.arg > work_register);
                    
                    if (is_equal) {
                        compare_flag |= PROG_CMP_EQUAL;
                    }
                    if (is_less) {
                        compare_flag |= PROG_CMP_LESS;
                    }
                    if (is_greater) {
                        compare_flag |= PROG_CMP_GREATER;
                    }
                }
                break;
                case OP_BRANCH_IF_EQUAL:
                    if (compare_flag & prog_compare_return_values::PROG_CMP_EQUAL) {
                        jump_to = prog.arg;
                        exec = false;
                    }
                break;
                case OP_BRANCH_IF_NOT_EQUAL:
                    if (!(compare_flag & prog_compare_return_values::PROG_CMP_EQUAL)) {
                        jump_to = prog.arg;
                        exec = false;
                    }
                break;
                case OP_BRANCH_IF_LESS:
                    if (compare_flag & prog_compare_return_values::PROG_CMP_LESS) {
                        jump_to = prog.arg;
                        exec = false;
                    }
                break;
                case OP_BRANCH_IF_GREATER:
                    if (compare_flag & prog_compare_return_values::PROG_CMP_GREATER) {
                        jump_to = prog.arg;
                        exec = false;
                    }
                break;
                case OP_SELECT_FLAG_208:
                    
                    // This opcode is a no-op in the original game, possibly reserved for future use.
                    
                break;
                case OP_DIST_TO_TARGET:
                    work_register = this->actor->getDistanceToTarget(prog.arg);
                break;
                case OP_DIST_TO_SPOT:
                    work_register = this->actor->getDistanceToSpot(prog.arg);
                break;
                case OP_IS_TARGET_ALIVE:
                    for (auto test_actor: this->mission->actors) {
                        if (test_actor->actor_id == prog.arg) {
                            if (test_actor->object->alive == true) {
                                compare_flag = prog_compare_return_values::PROG_CMP_NOT_EQUAL;
                            } else {
                                compare_flag = prog_compare_return_values::PROG_CMP_EQUAL;
                            }
                            break;
                        }
                    }
                break;
                case OP_IS_TARGET_ACTIVE:
                    for (auto test_actor: this->mission->actors) {
                        if (test_actor->actor_id == prog.arg) {
                            if ((test_actor->is_active) || (test_actor->actor_name == "PLAYER")) {
                                compare_flag = prog_compare_return_values::PROG_CMP_EQUAL;
                            } else {
                                compare_flag = prog_compare_return_values::PROG_CMP_NOT_EQUAL;
                            }
                            break;
                        }
                    }
                break;
                case OP_SET_FLAG_TO_TRUE:
                    this->mission->mission->mission_data.flags[prog.arg] = 1;
                break;
                case OP_SET_FLAG_TO_FALSE:
                    this->mission->mission->mission_data.flags[prog.arg] = 0;
                break;
                case OP_TEST_FLAG:
                    if (this->mission->mission->mission_data.flags[prog.arg] != 0) {
                        compare_flag = prog_compare_return_values::PROG_CMP_EQUAL;
                    } else {
                        compare_flag = prog_compare_return_values::PROG_CMP_NOT_EQUAL;
                    }
                break;
                case OP_MUL_VALUE_WITH_WORK:
                    work_register *= prog.arg;
                break;
                case OP_ACTIVATE_SCENE:
                {
                    int scene_id=0;
                    for (auto scen: this->mission->mission->mission_data.scenes) {
                        if (scene_id == prog.arg) {
                            scen->is_active = 1;
                            break;
                        }
                        scene_id++;
                    }
                }
                break;
                case OP_DEACTIVATE_SCENE:
                {
                    int scene_id=0;
                    for (auto scen: this->mission->mission->mission_data.scenes) {
                        if (scene_id == prog.arg) {
                            scen->is_active = 0;
                            break;
                        }
                        scene_id++;
                    }
                }
                break;
                default:
                break;
            }
        }
        i++;
    }
}