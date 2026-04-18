#include "fsm.h"

FSM::FSM() : currentState_(SET_BASE_TIME) {}

void FSM::processInput(int input) {
    // Simple state transitions based on input
    // 0: next state, 1: previous state, etc.
    switch (currentState_) {
        case SET_BASE_TIME:
            if (input == 0) currentState_ = SET_F_STOP;
            break;
        case SET_F_STOP:
            if (input == 0) currentState_ = CALCULATE_TIME;
            else if (input == 1) currentState_ = SET_BASE_TIME;
            break;
        case CALCULATE_TIME:
            if (input == 0) currentState_ = EXPOSE;
            else if (input == 1) currentState_ = SET_F_STOP;
            break;
        case EXPOSE:
            if (input == 1) currentState_ = CALCULATE_TIME;
            break;
    }
}

State FSM::getCurrentState() const {
    return currentState_;
}

void FSM::setState(State state) {
    currentState_ = state;
}