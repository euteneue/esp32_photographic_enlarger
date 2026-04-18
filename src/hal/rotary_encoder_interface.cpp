#include "rotary_encoder_interface.h"
#include <Arduino.h>

RotaryEncoderInterface::RotaryEncoderInterface(int clk, int dt, int sw) 
    : encoder_(clk, dt), swPin_(sw), lastButtonState_(false) {
    if (sw != -1) {
        pinMode(sw, INPUT_PULLUP);
    }
}

void RotaryEncoderInterface::tick() {
    encoder_.tick();
}

int RotaryEncoderInterface::getPosition() {
    return encoder_.getPosition();
}

RotaryEncoder::Direction RotaryEncoderInterface::getDirection() {
    return encoder_.getDirection();
}

bool RotaryEncoderInterface::buttonPressed() {
    if (swPin_ == -1) return false;
    bool current = digitalRead(swPin_) == LOW;
    bool pressed = !lastButtonState_ && current;
    lastButtonState_ = current;
    return pressed;
}