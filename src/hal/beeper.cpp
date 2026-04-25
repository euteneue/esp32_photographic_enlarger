#include "beeper.h"

// Frequency definitions (in Hz)
#define TICK_FREQ 1000
#define HIGH_FREQ 2000
#define LOW_FREQ 500

// Duration definitions (in milliseconds)
#define TICK_DURATION 10
#define BEEP_DURATION 200
#define PAUSE_DURATION 100

Beeper::Beeper(int pin, int channel) : pin_(pin), channel_(channel) {
    // Setup PWM channel with 2000 Hz frequency and 8-bit resolution
    ledcSetup(channel_, 2000, 8);
    ledcAttachPin(pin_, channel_);
}

void Beeper::tick() {
    ledcWriteTone(channel_, TICK_FREQ);
    delay(TICK_DURATION);
    ledcWriteTone(channel_, 0);  // Stop the tone
}

void Beeper::highBeep() {
    ledcWriteTone(channel_, HIGH_FREQ);
    delay(BEEP_DURATION);
    ledcWriteTone(channel_, 0);
}

void Beeper::lowBeep() {
    ledcWriteTone(channel_, LOW_FREQ);
    delay(BEEP_DURATION);
    ledcWriteTone(channel_, 0);
}

void Beeper::doubleBeep() {
    // First beep
    ledcWriteTone(channel_, TICK_FREQ);
    delay(BEEP_DURATION);
    ledcWriteTone(channel_, 0);
    delay(PAUSE_DURATION);
    // Second beep
    ledcWriteTone(channel_, TICK_FREQ);
    delay(BEEP_DURATION);
    ledcWriteTone(channel_, 0);
}