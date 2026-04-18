#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

/**
 * @brief Class for controlling a relay module
 *
 * This class provides an interface to control a relay connected to a digital pin
 * on the ESP32. It handles turning the relay on/off and tracking its current state.
 */
class Relay {
public:
    /**
     * @brief Construct a new Relay object
     * @param pin The GPIO pin number connected to the relay
     */
    Relay(int pin);

    /**
     * @brief Turn the relay on
     */
    void on();

    /**
     * @brief Turn the relay off
     */
    void off();

    /**
     * @brief Check if the relay is currently on
     * @return true if relay is on, false otherwise
     */
    bool isOn() const;

private:
    int pin_;      /**< GPIO pin number */
    bool state_;   /**< Current relay state (true = on, false = off) */
};

#endif