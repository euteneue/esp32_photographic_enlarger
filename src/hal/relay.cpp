#include "relay.h"

Relay::Relay(int pin) : pin_(pin), state_(false) {
    pinMode(pin_, OUTPUT);
    off();
}

void Relay::on() {
    digitalWrite(pin_, HIGH);
    state_ = true;
}

void Relay::off() {
    digitalWrite(pin_, LOW);
    state_ = false;
}

bool Relay::isOn() const {
    return state_;
}