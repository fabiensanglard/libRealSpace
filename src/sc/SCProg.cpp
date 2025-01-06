#include "precomp.h"

void SCProg::execute() {
    size_t i = 0;
    size_t jump_to = 0;
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
                    jump_to = prog.arg;
                    exec = false;
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
                true_flag = true;
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
            default:
            break;
        }
    }
}