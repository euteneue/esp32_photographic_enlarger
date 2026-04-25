#ifndef TM1638_INTERFACE_H
#define TM1638_INTERFACE_H

#include <TM1638plus.h>
#include "../logic/exposure_status.h"


/**
 * @brief Interface class for TM1638 LED display module
 *
 * This class provides an interface to control the TM1638 7-segment display
 * and LED module, including display text, brightness control, and button reading.
 */
class TM1638Interface {
public:
    /**
     * @brief Construct a new TM1638Interface object
     * @param dio DIO pin for TM1638 communication
     * @param clk CLK pin for TM1638 communication
     * @param stb STB pin for TM1638 communication
     * @param mutex Semaphore handle for synchronizing access to TM1638 (must be created by caller)
     */
    TM1638Interface(int dio, int clk, int stb, SemaphoreHandle_t mutex);

    /**
     * @brief Clear the display
     */
    void clear();

    /**
     * @brief Set the display brightness
     * @param brightness Brightness level (0-7)
     */
    void setBrightness(uint8_t brightness);

    /**
     * @brief Display a string on the 7-segment display
     * @param str String to display (up to 8 characters)
     */
    void setDisplay(const char* str);

    /**
     * @brief Set the state of the LEDs
     * @param leds LED bitmask (8 bits for 8 LEDs)
     */
    void setLEDs();

    /**
     * @brief Set the state of the LEDs, without affecting the display. Call setLEDs to update the display with the new LED state after calling this method.
     * @param leds LED bitmask (8 bits for 8 LEDs)
     */
    void setLEDState(uint8_t leds);

    /**
     * @brief Get the current state of the LEDs
     * @return LED bitmask (8 bits for 8 LEDs)
     */
    uint8_t getLEDState() const;

    /**
     * @brief Read the state of the buttons
     * @return Button bitmask (8 bits for 8 buttons)
     */
    uint8_t getButtons();



private:
    TM1638plus tm1638_; /**< TM1638plus instance for hardware control */
    char displayBuffer_[MAX_DISPLAY_STR_LEN+1]; /**< Buffer for display text (8 characters + eventual decimal points + null terminator) */
    uint8_t lastButtonsState_; /**< Last read state of the buttons, used for debouncing and change detection */
    uint8_t currentButtonsState_; /**< Current state of the buttons */
    unsigned long lastButtonReadTime_; /**< Timestamp of the last button read, used for debouncing */
    SemaphoreHandle_t tm1638Mutex_; /**< Mutex to protect access to TM1638 display and buttons */

    uint8_t ledState_; /**< Current state of the LEDs, stored as a bitmask */
};

#endif