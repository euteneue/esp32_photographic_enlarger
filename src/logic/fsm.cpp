#include "fsm.h"

// FSM::FSM() : currentState_(INITIAL), testStripStep_(0) {}

// void FSM::processInput(MsgType event) {
//     switch (currentState_) {
//         case INITIAL:
//             if (event == MsgType::MODE_BUTTON_PRESS) {
//                 currentState_ = FOCUS_LIGHT_OFF;
//             }
//             break;
//         case FOCUS_LIGHT_OFF:
//             if (event == MsgType::BUTTON_MODE_PRESS) {
//                 currentState_ = FOCUS_LIGHT_ON;
//             } else if (event == MsgType::MODE_BUTTON_PRESS) {
//                 currentState_ = TEST_STRIP_CONFIG;
//             }
//             break;
//         case FOCUS_LIGHT_ON:
//             if (event == MsgType::BUTTON_MODE_PRESS) {
//                 currentState_ = FOCUS_LIGHT_OFF;
//             }
//             break;
//         case TEST_STRIP_CONFIG:
//             if (event == MsgType::BUTTON_MODE_PRESS) {
//                 currentState_ = TEST_STRIP_SEQUENCE;
//                 testStripStep_ = -3;
//             } else if (event == MsgType::MODE_BUTTON_PRESS) {
//                 currentState_ = FSTOP_EXPOSURE_CONFIG;
//             }
//             break;
//         case TEST_STRIP_SEQUENCE:
//             // Events handled externally for sequence advancement
//             break;
//         case FSTOP_EXPOSURE_CONFIG:
//             if (event == MsgType::BUTTON_MODE_PRESS) {
//                 currentState_ = FSTOP_EXPOSURE;
//             } else if (event == MsgType::MODE_BUTTON_PRESS) {
//                 currentState_ = TIME_EXPOSURE_CONFIG;
//             }
//             break;
//         case FSTOP_EXPOSURE:
//             // Automatic transition back handled externally
//             break;
        
//         case TIME_EXPOSURE_CONFIG:
//             if (event == MsgType::BUTTON_MODE_PRESS) {
//                 currentState_ = TIME_EXPOSURE;
//             } else if (event == MsgType::MODE_BUTTON_PRESS) {
//                 currentState_ = FOCUS_LIGHT_OFF;
//             }
//             break;
//         case TIME_EXPOSURE:
//             // Automatic transition back handled externally
//             break;  
//     }
// }

// State FSM::getCurrentState() const {
//     return currentState_;
// }

// void FSM::setState(State state) {
//     currentState_ = state;
// }

// int FSM::getTestStripStep() const {
//     return testStripStep_;
// }

// void FSM::advanceTestStrip() {
//     testStripStep_++;
//     if (testStripStep_ > 3) {
//         currentState_ = TEST_STRIP_CONFIG;
//         testStripStep_ = 0; // Reset for next time
//     }
// }