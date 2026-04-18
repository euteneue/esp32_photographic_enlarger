#ifndef ROTARY_ENCODER_INTERFACE_H
#define ROTARY_ENCODER_INTERFACE_H

#include <RotaryEncoder.h>

/**
 * @brief Interface class for rotary encoder with optional button
 *
 * This class wraps the RotaryEncoder library to provide a simplified interface
 * for reading rotary encoder position, direction, and button presses.
 */
class RotaryEncoderInterface {
public:
    /**
     * @brief Construct a new RotaryEncoderInterface object
     * @param clk Clock pin of the rotary encoder
     * @param dt Data pin of the rotary encoder
     * @param sw Switch/button pin of the rotary encoder (-1 if no button)
     */
    RotaryEncoderInterface(int clk, int dt, int sw = -1);

    /**
     * @brief Update the encoder state (call this frequently in loop)
     */
    void tick();

    /**
     * @brief Get the current position of the encoder
     * @return Current encoder position
     */
    int getPosition();

    /**
     * @brief Get the direction of the last rotation
     * @return Direction enum (NODIR, CW, CCW)
     */
    RotaryEncoder::Direction getDirection();

    /**
     * @brief Check if the button was pressed since last check
     * @return true if button was pressed, false otherwise
     */
    bool buttonPressed();

private:
    RotaryEncoder encoder_;           /**< RotaryEncoder instance */
    int swPin_;                       /**< Switch pin number (-1 if no button) */
    bool lastButtonState_;            /**< Last button state for edge detection */
};

#endif