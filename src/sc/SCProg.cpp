#include "precomp.h"

void SCProg::execute() {
    size_t i = 0;
    size_t jump_to = 0;
    bool exec = true;
    for (auto prog : this->prog) {
        if (jump_to != 0) {
            if (i != jump_to) {
                i++;
                continue;
            } else {
                jump_to = 0;
            }
        }
        switch (prog.opcode) {
            case OP_EXIT_PROG:
                if (exec) {
                    return;
                }
            break;
            case OP_SET_LABEL:
                if (jump_to == prog.arg) {
                    exec = true;
                }
            break;
            case OP_GOTO_LABEL:
                if (exec) {
                    jump_to = this->labels[prog.arg];
                    exec = false;
                }
            break;
            case OP_IF_TARGET_IN_AREA:
                
            break;
            case OP_INSTANT_DESTROY_TARGET:
            break;
            case OP_SET_OBJ_TAKE_OFF:
                if (exec) {
                    this->actor->takeOff(prog.arg);
                }
            break;
            case OP_SET_OBJ_LAND:
                if (exec) {
                    this->actor->land(prog.arg);
                }
            break;
            case OP_SET_OBJ_FLY_TO_WP:
                if (exec) {
                    this->actor->flyToWaypoint(prog.arg);
                }
            break;
            case OP_SET_OBJ_FLY_TO_AREA:
                if (exec) {
                    this->actor->flyToArea(prog.arg);
                }
            break;
            case OP_SET_OBJ_DESTROY_TARGET:
                if (exec) {
                    this->actor->destroyTarget(prog.arg);
                }
            break;
            case OP_SET_OBJ_DEFEND_TARGET:
                if (exec) {
                    this->actor->defendTarget(prog.arg);
                }
            break;
            case OP_SET_MESSAGE:
                if (exec) {
                    this->actor->setMessage(prog.arg);
                }
            break;
            case OP_DEACTIVATE_OBJ:
                if (exec) {
                    this->actor->deactivate(prog.arg);
                }
            break;
            default:
            break;
        }
    }
}