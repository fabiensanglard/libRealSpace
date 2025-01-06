#include "precomp.h"

void SCProg::execute() {
    size_t i = 0;
    size_t jump_to = 0;
    size_t call_to = 0;
    int flag_number = 0;
    bool exec = true;
    bool objective_flag = false;
    bool true_flag = false;
    for (auto prog : this->prog) {
        switch (prog.opcode) {
            case OP_EXIT_PROG:
                if (exec) {
                    if (true_flag) {
                        true_flag = false;
                    } else {
                        return;
                    }
                }
            break;
            case OP_SET_LABEL:
                if (jump_to == prog.arg) {
                    exec = true;
                }
            break;
            case OP_CALL_LABEL:
                if (exec) {
                    call_to = prog.arg;
                }
            break;
            case OP_GOTO_LABEL_IF_TRUE:
                if (exec) {
                    if (objective_flag) {
                        jump_to = prog.arg;
                        exec = false;
                    }
                }
            break;
            case OP_GOTO_LABEL_IF_FALSE:
                if (exec) {
                    if (!objective_flag) {
                        jump_to = prog.arg;
                        exec = false;
                    }
                }
            break;
            case OP_IF_TARGET_IN_AREA:
                if (exec) {
                    objective_flag = this->actor->ifTargetInSameArea(prog.arg);
                }
            break;
            case OP_85_UNKNOWN:
                if (exec) {
                    true_flag = this->mission->mission->mission_data.flags[prog.arg] != 0;
                }
                
            break;
            case OP_INSTANT_DESTROY_TARGET:
            break;
            case OP_SET_OBJ_TAKE_OFF:
                if (exec) {
                    objective_flag = this->actor->takeOff(prog.arg);
                }
            break;
            case OP_SET_OBJ_LAND:
                if (exec) {
                    objective_flag = this->actor->land(prog.arg);
                }
            break;
            case OP_SET_OBJ_FLY_TO_WP:
                if (exec) {
                    objective_flag = this->actor->flyToWaypoint(prog.arg);
                }
            break;
            case OP_SET_OBJ_FLY_TO_AREA:
                if (exec) {
                    objective_flag = this->actor->flyToArea(prog.arg);
                }
            break;
            case OP_SET_OBJ_DESTROY_TARGET:
                if (exec) {
                    objective_flag = this->actor->destroyTarget(prog.arg);
                }
            break;
            case OP_SET_OBJ_DEFEND_TARGET:
                if (exec) {
                    objective_flag = this->actor->defendTarget(prog.arg);
                }
            break;
            case OP_SET_MESSAGE:
                if (exec) {
                    objective_flag = this->actor->setMessage(prog.arg);
                }
            break;
            case OP_SET_OBJ_FOLLOW_ALLY:
                if (exec) {
                    objective_flag = this->actor->followAlly(prog.arg);
                }
            break;
            case OP_DEACTIVATE_OBJ:
                if (exec) {
                    objective_flag = this->actor->deactivate(prog.arg);
                }
            break;
            case OP_ACTIVATE_OBJ:
                if (exec) {
                    this->actor->activateTarget(prog.arg);
                }
            case OP_SELECT_FLAG:
                if (exec) {
                    flag_number = prog.arg;
                }
            break;
            case OP_SAVE_VALUE_TO_FLAG:
                if (exec) {
                    this->mission->mission->mission_data.flags[flag_number] = prog.arg;
                }
            break;
            case OP_STORE_CALL_TO_VALUE:
                if (exec) {
                    this->mission->mission->mission_data.flags[prog.arg] = (uint8_t) call_to;
                }
            break;
            case OP_EXECUTE_CALL:
                if (exec) {
                    jump_to = this->mission->mission->mission_data.flags[flag_number];
                    exec = false;
                }
            break;
            default:
            break;
        }
    }
}