#ifndef EXPOSURE_TIMER_H
#define EXPOSURE_TIMER_H

#include "relay.h"
#include "rotary_encoder_interface.h"
#include "tm1638_interface.h"
#include <Arduino.h>
#include <AiEsp32RotaryEncoder.h>

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
     * @return Reference to the singleton ExposureTimer instance
     */
    static ExposureTimer& getInstance(ExposureStatus* status = nullptr, Relay* relay = nullptr, 
                                     AiEsp32RotaryEncoder* encoderValue = nullptr, 
                                     AiEsp32RotaryEncoder* encoderMode = nullptr, 
                                     TM1638Interface* display = nullptr);


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
     * @brief Start an exposure with the given duration
     * @param durationMs Duration of exposure in milliseconds
     */
    void start(float durationMs);

    /**
     * @brief Update the exposure timer (call periodically from task)
     * @return true if exposure just completed, false otherwise
     */
    bool tick();

    /**
     * @brief Check if exposure is currently in progress
     * @return true if exposing, false otherwise
     */
    bool isExposing() const;

    /**
     * @brief Get the remaining exposure time in milliseconds
     * @return Remaining time in milliseconds
     */
    float getRemainingTime() const;

    /**
     * @brief Cancel the current exposure
     */
    void cancel();

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
     * @brief Get pointer to the exposure status instance
     * @return Pointer to the ExposureStatus object
     */
    ExposureStatus* getStatus() const;


    /**
     * @brief Process user input and transition states accordingly
     * @param event User event
     */
    void processInput(MsgType event);

private:
    /**
     * @brief Construct a new ExposureTimer object
     * @param status Pointer to the ExposureStatus object to track current status
     * @param relay Pointer to the Relay instance to control
     * @param encoderValue Pointer to the rotary encoder instance for value adjustment
     * @param encoderMode Pointer to the rotary encoder instance for mode adjustment
     * @param display Pointer to the TM1638 display instance
     */
    ExposureTimer(ExposureStatus* status, Relay* relay, AiEsp32RotaryEncoder* encoderValue, AiEsp32RotaryEncoder* encoderMode, TM1638Interface* display);

    static ExposureTimer* instance_; /**< Static singleton instance pointer */

    ExposureStatus* status_;                    /**< Pointer to exposure status */
    Relay* relay_;                              /**< Pointer to relay controller */
    AiEsp32RotaryEncoder* encoderValue_;        /**< Pointer to value adjustment encoder */
    AiEsp32RotaryEncoder* encoderMode_;         /**< Pointer to mode adjustment encoder */
    TM1638Interface* display_;                  /**< Pointer to display interface */
    bool exposing_;                             /**< Flag indicating if exposure is active */
    float remainingTimeMs_;                     /**< Remaining exposure time in milliseconds */
    unsigned long lastTickMs_;                  /**< Timestamp of last tick for timing calculations */
 

    static void IRAM_ATTR readEncoderModeISR();
    static void IRAM_ATTR readEncoderValueISR();

};

#endif // EXPOSURE_TIMER_H
