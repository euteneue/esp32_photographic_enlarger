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
     */
    TM1638Interface(int dio, int clk, int stb);

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
    void setLEDs(uint8_t leds);

    /**
     * @brief Read the state of the buttons
     * @return Button bitmask (8 bits for 8 buttons)
     */
    uint8_t getButtons();

    /**
     * @brief Display both time and step information
     * @param status ExposureStatus containing time and step data
     */
    void displayTimeandStep(ExposureStatus status);

    /**
     * @brief Display only the exposure time
     * @param status ExposureStatus containing time data
     */
    void displayTime(ExposureStatus status);

    /**
     * @brief Display only the step information
     * @param status ExposureStatus containing step data
     */
    void displayStep(ExposureStatus status);

    /**
     * @brief Display the current mode (TestStrip or Exposure)
     * @param status ExposureStatus containing mode data
     */
    void displayMode(ExposureStatus status);

private:
    TM1638plus tm1638_; /**< TM1638plus instance for hardware control */
};

#endif