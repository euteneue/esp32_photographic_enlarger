#ifndef FSM_H
#define FSM_H

#include "config.h"



// /**
//  * @brief Finite State Machine for enlarger control
//  *
//  * This class implements a finite state machine to manage the
//  * different operational states of the photographic enlarger.
//  */
// class FSM {
// public:
//     /**
//      * @brief Construct a new FSM object
//      */
//     FSM();

//     /**
//      * @brief Process user input and transition states accordingly
//      * @param event User event
//      */
//     void processInput(MsgType event);

//     /**
//      * @brief Get the current state of the FSM
//      * @return Current State enum value
//      */
//     State getCurrentState() const;

//     /**
//      * @brief Set the current state of the FSM
//      * @param state New state to transition to
//      */
//     void setState(State state);

//     /**
//      * @brief Get the current test strip step (-3 to 3)
//      * @return Current test strip step
//      */
//     int getTestStripStep() const;

//     /**
//      * @brief Advance to the next test strip step
//      */
//     void advanceTestStrip();

// private:
//     State currentState_;     /**< Current state of the finite state machine */
//     int testStripStep_;      /**< Current step in test strip sequence (-3 to 3) */
// };

#endif