#include "SCProg.h"

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
            break;
            case OP_SET_OBJ_LAND:
            break;
            case OP_SET_OBJ_FLY_TO_WP:
            break;
            case OP_SET_OBJ_FLY_TO_AREA:
            break;
            case OP_SET_OBJ_DESTROY_TARGET:
            break;
            case OP_SET_OBJ_DEFEND_TARGET:
            break;
            case OP_SET_MESSAGE:
            break;
            case OP_DEACTIVATE_OBJ:
                if (exec) {
                    this->actor->profile->ai.isAI = false;
                }
            break;
            default:
            break;
        }
    }
}