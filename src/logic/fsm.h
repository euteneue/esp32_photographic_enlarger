#ifndef FSM_H
#define FSM_H

#include "config.h"

/**
 * @brief States for the finite state machine
 */
enum State {
    INITIAL,                /**< Initial state with welcome message */
    FOCUS_LIGHT_OFF,        /**< Focus light is off */
    FOCUS_LIGHT_ON,         /**< Focus light is on */
    TEST_STRIP_CONFIG,      /**< Configuring test strip parameters */
    TEST_STRIP_SEQUENCE,    /**< Running test strip exposure sequence */
    FSTOP_EXPOSURE_CONFIG,  /**< Configuring f-stop exposure parameters */
    FSTOP_EXPOSURE,          /**< Performing f-stop exposure */
    TIME_EXPOSURE_CONFIG,   /**< Configuring time-based exposure parameters */
    TIME_EXPOSURE           /**< Performing time-based exposure */
};

/**
 * @brief Finite State Machine for enlarger control
 *
 * This class implements a finite state machine to manage the
 * different operational states of the photographic enlarger.
 */
class FSM {
public:
    /**
     * @brief Construct a new FSM object
     */
    FSM();

    /**
     * @brief Process user input and transition states accordingly
     * @param event User event
     */
    void processInput(MsgType event);

    /**
     * @brief Get the current state of the FSM
     * @return Current State enum value
     */
    State getCurrentState() const;

    /**
     * @brief Set the current state of the FSM
     * @param state New state to transition to
     */
    void setState(State state);

    /**
     * @brief Get the current test strip step (-3 to 3)
     * @return Current test strip step
     */
    int getTestStripStep() const;

    /**
     * @brief Advance to the next test strip step
     */
    void advanceTestStrip();

private:
    State currentState_;     /**< Current state of the finite state machine */
    int testStripStep_;      /**< Current step in test strip sequence (-3 to 3) */
};

#endif