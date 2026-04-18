#ifndef FSM_H
#define FSM_H

/**
 * @brief States for the finite state machine
 */
enum State {
    SET_BASE_TIME,   /**< Setting the base exposure time */
    SET_F_STOP,      /**< Setting the f-stop value */
    CALCULATE_TIME,  /**< Calculating the final exposure time */
    EXPOSE           /**< Performing the exposure */
};

/**
 * @brief Finite State Machine for enlarger control
 *
 * This class implements a simple finite state machine to manage the
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
     * @param input User input (0 = button press, 1/-1 = encoder rotation)
     */
    void processInput(int input);

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

private:
    State currentState_; /**< Current state of the finite state machine */
};

#endif