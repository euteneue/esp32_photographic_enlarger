#ifndef EXPOSURE_TIMER_H
#define EXPOSURE_TIMER_H

#include "relay.h"
#include "rotary_encoder_interface.h"
#include "tm1638_interface.h"
#include <Arduino.h>
#include <AiEsp32RotaryEncoder.h>
#include <Bounce2.h>
#include "beeper.h"
#include "config.h"

/**
 * @brief Class for managing photographic exposure timing
 *
 * This class handles the timing and control of photographic exposures,
 * including relay control and exposure duration management.
 * Implemented as a singleton to ensure only one instance exists.
 */
class ExposureTimer {
public:
    /**
     * @brief Get the singleton instance of ExposureTimer
     * @param status Pointer to the ExposureStatus object to track current status
     * @param relay Pointer to the Relay instance to control
     * @param encoderValue Pointer to the rotary encoder instance for value adjustment
     * @param encoderMode Pointer to the rotary encoder instance for mode adjustment
     * @param display Pointer to the TM1638 display instance
     * @param beeper Beeper instance for audio feedback
     * @param footSwitch Pointer to the foot switch button instance
     * @param inputQueue Queue handle for simulating input events from state manager
     * @param displayQueue Queue handle for sending display update messages to display manager
     * @return Reference to the singleton ExposureTimer instance
     */
    static ExposureTimer& getInstance(ExposureStatus* status = nullptr, Relay* relay = nullptr, 
                                     AiEsp32RotaryEncoder* encoderValue = nullptr, 
                                     AiEsp32RotaryEncoder* encoderMode = nullptr, 
                                     TM1638Interface* display = nullptr,
                                     Beeper* beeper = nullptr, 
                                     Bounce2::Button* footSwitch = nullptr,
                                     QueueHandle_t inputQueue = nullptr, 
                                     QueueHandle_t displayQueue = nullptr,
                                     QueueHandle_t beeperQueue = nullptr);


    /**
     * @brief Destroy the ExposureTimer object
     *
     * Automatically cancels any active exposure when the object is destroyed.
     */
    ~ExposureTimer();

    // Delete copy constructor and assignment operator
    ExposureTimer(const ExposureTimer&) = delete;
    ExposureTimer& operator=(const ExposureTimer&) = delete;

    /**
     * @brief  Initialize the exposure timer (call once in setup)
     * 
     */
    void setup();

    /**
     * @brief Get pointer to the relay instance
     * @return Pointer to the Relay object
     */
    Relay* getRelay() const;

    /**
     * @brief Get pointer to the value encoder instance
     * @return Pointer to the AiEsp32RotaryEncoder object for value adjustment
     */
    AiEsp32RotaryEncoder* getEncoderValue() const;

    /**
     * @brief Get pointer to the mode encoder instance
     * @return Pointer to the AiEsp32RotaryEncoder object for mode adjustment
     */
    AiEsp32RotaryEncoder* getEncoderMode() const;

    /**
     * @brief Get pointer to the display instance
     * @return Pointer to the TM1638Interface object
     */
    TM1638Interface* getDisplay() const;

    /**
     * @brief Get the Beeper instance for audio feedback
     * @return Beeper object
     */
    Beeper* getBeeper() const;

    /**
     * @brief Get pointer to the exposure status instance
     * @return Pointer to the ExposureStatus object
     */
    ExposureStatus* getStatus() const;

    /**
     * @brief Process user input and transition states accordingly
     * @param event User event
     * @param payload Optional pointer to additional data (for example encoder value)
     */
    void processInput(MsgType event, void *payload = nullptr);

    /**
     * @brief Called by the ExposureTimer task to manage the exposure timing and control the relay. 
     * This method checks if the exposure time has elapsed, turns off the relay if needed, and updates 
     * the display with the remaining time. It also handles sending messages to the beeper manager for 
     * audio feedback during the exposure.
     * 
     */
    void exposureControl();

    /**
     * @brief Handle user input from the TM1638 buttons and rotary encoders. This method checks the state 
     * of the buttons and encoders, and sends appropriate messages to the state manager to update the 
     * system state based on user interactions. 
     * 
     */
    void handleInput();

    /**
     * @brief Handle the event queue for the beeper manager. This method checks for messages related to beeper 
     * events (for example, tick, high beep, low beep, double beep) and calls the corresponding methods on the 
     * Beeper instance to produce the appropriate sounds.
     * 
     */
    void handleBeeper(QueueItem *msg);

    /**
     * @brief Get the Display Buffer object for formatting display strings
     * 
     * @return char* Pointer to the display buffer
     */
    const char *getDisplayBuffer() const;

    void displayExposingTime(unsigned long timeMs);    

private:
    /**
     * @brief Construct a new ExposureTimer object
     * @param status Pointer to the ExposureStatus object to track current status
     * @param relay Pointer to the Relay instance to control
     * @param encoderValue Pointer to the rotary encoder instance for value adjustment
     * @param encoderMode Pointer to the rotary encoder instance for mode adjustment
     * @param display Pointer to the TM1638 display instance
     * @param beeper Beeper instance for audio feedback
     * @param footSwitch Pointer to the foot switch button instance
     * @param inputQueue Queue handle for simulating input events from state manager
     * @param displayQueue Queue handle for sending display update messages to display manager
     * @param beeperQueue Queue handle for sending beeper update messages to beeper manager
     */
    ExposureTimer(ExposureStatus* status, Relay* relay, AiEsp32RotaryEncoder* encoderValue, AiEsp32RotaryEncoder* encoderMode, TM1638Interface* display, Beeper* beeper, Bounce2::Button* footSwitch, QueueHandle_t inputQueue, QueueHandle_t displayQueue, QueueHandle_t beeperQueue);

    void displayMode();

    void displayTimeandGranularity();

    void displayTimeandStep();

    void displayTime();

    void displayStep();

    void displayMessage(const char* message);

    void countdownExposureTime(double exposureTime);    

    static ExposureTimer* instance_; /**< Static singleton instance pointer */

    ExposureStatus* status_;                    /**< Pointer to exposure status */
    Relay* relay_;                              /**< Pointer to relay controller */
    AiEsp32RotaryEncoder* encoderValue_;        /**< Pointer to value adjustment encoder */
    AiEsp32RotaryEncoder* encoderMode_;         /**< Pointer to mode adjustment encoder */
    TM1638Interface* display_;                  /**< Pointer to display interface */
    Beeper* beeper_;                            /**< Beeper instance for audio feedback */  
    Bounce2::Button* footSwitch_;                /**< Pointer to foot switch button instance */
    
    bool exposing_;                             /**< Flag indicating if exposure is active */
    float remainingTimeMs_;                     /**< Remaining exposure time in milliseconds */
    unsigned long lastTickMs_;                  /**< Timestamp of last tick for timing calculations */
    QueueItem msg_;                              /**< Queue item for sending messages from state manager, e.g. to the DisplayManager or the ExposureTimer */
    long valueBuffer_;                            /**< Buffer for encoder value to be sent in queue messages */
    // Queues for inter-task communication
    QueueHandle_t inputQueue_;                  /**< Queue for simulating input events from state manager */
    QueueHandle_t displayQueue_;                /**< Queue for sending display update messages to display manager */   
    QueueHandle_t beeperQueue_;                 /**< Queue for sending beeper update messages to beeper manager */

    char displayBuffer_[MAX_DISPLAY_STR_LEN+1]; /**< Buffer for formatting display strings */
 

    static void IRAM_ATTR readEncoderModeISR();
    static void IRAM_ATTR readEncoderValueISR();

    void refreshDisplay();

};

#endif // EXPOSURE_TIMER_H
