#include "SCProg.h"

void SCProg::parse() {
    size_t i = 0;
    for (auto prog : this->prog) {
        switch (prog.opcode) {
            case OP_EXIT_PROG:
            break;
            case OP_SET_LABEL:
                this->labels[prog.arg] = i;
            break;
            case OP_GOTO_LABEL:
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
            break;
            default:
            break;
        }
        i++;
    }
}

void SCProg::execute() {
    size_t i = 0;
    size_t jump_to = 0;
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
                return;
            break;
            case OP_SET_LABEL:
            break;
            case OP_GOTO_LABEL:
                jump_to = this->labels[prog.arg];
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
                this->actor->profile->ai.isAI = false;
            break;
            default:
            break;
        }
    }
}