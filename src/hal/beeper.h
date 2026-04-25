#ifndef BEEPER_H
#define BEEPER_H

#include <Arduino.h>

/**
 * @brief Class for controlling a piezo buzzer/beeper
 *
 * This class provides an interface to control a piezo buzzer connected to a PWM-capable
 * GPIO pin on the ESP32. It can produce different types of beeps and tones.
 */
class Beeper {
public:
    /**
     * @brief Construct a new Beeper object
     * @param pin The GPIO pin number connected to the buzzer
     * @param channel The PWM channel to use (0-15, default 0)
     */
    Beeper(int pin, int channel = 0);

    /**
     * @brief Produce a short tick sound
     */
    void tick();

    /**
     * @brief Produce a high-pitched beep
     */
    void highBeep();

    /**
     * @brief Produce a low-pitched beep
     */
    void lowBeep();

    /**
     * @brief Produce a double beep (two beeps in succession)
     */
    void doubleBeep();

private:
    int pin_;      /**< GPIO pin number */
    int channel_;  /**< PWM channel number */
};

#endif