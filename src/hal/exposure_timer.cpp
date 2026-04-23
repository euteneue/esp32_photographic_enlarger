#include "exposure_timer.h"
#include <Arduino.h>

// Initialize static instance
ExposureTimer* ExposureTimer::instance_ = nullptr;

ExposureTimer& ExposureTimer::getInstance(ExposureStatus* status, Relay* relay, 
                                         AiEsp32RotaryEncoder* encoderValue, 
                                         AiEsp32RotaryEncoder* encoderMode, 
                                         TM1638Interface* display) {
    if (instance_ == nullptr) {
        // Create instance only if it doesn't exist and we have all required parameters
        if (status && relay && encoderValue && encoderMode && display) {
            instance_ = new ExposureTimer(status, relay, encoderValue, encoderMode, display);
        } else {
            // If instance doesn't exist but parameters are null, this is an error
            // In a real implementation, you might want to handle this differently
            // For now, we'll assume getInstance is called with parameters on first call
        }
    }
    return *instance_;
}


ExposureTimer::ExposureTimer(ExposureStatus* status, Relay* relay, AiEsp32RotaryEncoder* encoderValue, AiEsp32RotaryEncoder* encoderMode, TM1638Interface* display)
    : status_(status), relay_(relay), encoderValue_(encoderValue), encoderMode_(encoderMode), display_(display), exposing_(false), remainingTimeMs_(0), lastTickMs_(0) 
{

}

ExposureTimer::~ExposureTimer() 
{
    cancel();

    // Note: In a singleton pattern, we typically don't delete the owned objects
    // here as they might be used elsewhere. The singleton instance itself
    // should be cleaned up explicitly if needed.
    // delete status_;
    // delete relay_;
    // delete encoderValue_;
    // delete encoderMode_;
    // delete display_;
}

void ExposureTimer::setup() 
{
     //we must initialize rotary encoder
    encoderMode_->begin();
    encoderValue_->begin();

    encoderMode_->setup(readEncoderModeISR);
    encoderValue_->setup(readEncoderValueISR);

    //set boundaries and if values should cycle or not
    //in this example we will set possible values between 0 and 1000;
    
    encoderMode_->setBoundaries(0, 1, true); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
    encoderValue_->setBoundaries(0, 999, true); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)

    encoderMode_->setAcceleration(0); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration
    encoderValue_->setAcceleration(250); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration

    display_->setBrightness(0);
}


void IRAM_ATTR ExposureTimer::readEncoderModeISR() 
{
    if (instance_ != nullptr) {
        instance_->encoderMode_->readEncoder_ISR();
    }
}

void IRAM_ATTR ExposureTimer::readEncoderValueISR() 
{
    if (instance_ != nullptr) {
        instance_->encoderValue_->readEncoder_ISR();
    }
}

Relay* ExposureTimer::getRelay() const
{
    return relay_;
}

AiEsp32RotaryEncoder* ExposureTimer::getEncoderValue() const
{
    return encoderValue_;
}

AiEsp32RotaryEncoder* ExposureTimer::getEncoderMode() const
{
    return encoderMode_;
}

TM1638Interface* ExposureTimer::getDisplay() const
{
    return display_;
}

ExposureStatus* ExposureTimer::getStatus() const
{
    return status_;
}


void ExposureTimer::start(float durationMs) 
{
    relay_->on();
    exposing_ = true;
    remainingTimeMs_ = durationMs;
    lastTickMs_ = millis();
}

bool ExposureTimer::tick() {
    if (!exposing_) {
        return false;
    }

    unsigned long currentTimeMs = millis();
    unsigned long elapsedMs = currentTimeMs - lastTickMs_;
    lastTickMs_ = currentTimeMs;

    remainingTimeMs_ -= elapsedMs;

    if (remainingTimeMs_ <= 0) {
        relay_->off();
        exposing_ = false;
        remainingTimeMs_ = 0;
        return true;  // Exposure just completed
    }

    return false;
}

bool ExposureTimer::isExposing() const {
    return exposing_;
}

float ExposureTimer::getRemainingTime() const {
    return remainingTimeMs_;
}

void ExposureTimer::cancel() {
    if (exposing_) {
        relay_->off();
        exposing_ = false;
        remainingTimeMs_ = 0;
    }
}

/**
 * @brief Process input events and update the exposure timer state accordingly
 * 
 * @param event The input event to process
 */
void ExposureTimer::processInput(MsgType event) {
    switch (status_->getMode()) {
        case State::INITIAL:
            if (event == MsgType::MODE_BUTTON_PRESS) {
                status_->setMode(State::FOCUS_LIGHT_OFF);
            }
            break;
        case State::FOCUS_LIGHT_OFF:
            if (event == MsgType::BUTTON_MODE_PRESS) {
                status_->setMode(State::FOCUS_LIGHT_ON);
            } else if (event == MsgType::MODE_BUTTON_PRESS) {
                status_->setMode(State::TEST_STRIP_CONFIG);
            }
            break;
        case State::FOCUS_LIGHT_ON:
            if (event == MsgType::BUTTON_MODE_PRESS) {
                status_->setMode(State::FOCUS_LIGHT_OFF);
            }
            break;
        case State::TEST_STRIP_CONFIG:
            if (event == MsgType::BUTTON_MODE_PRESS) {
                status_->setMode(State::TEST_STRIP_SEQUENCE);
                status_->setStep(MIN_STEP);
            } else if (event == MsgType::MODE_BUTTON_PRESS) {
                status_->setMode(State::FSTOP_EXPOSURE_CONFIG);
            }
            break;
        case State::TEST_STRIP_SEQUENCE:
            // Events handled externally for sequence advancement
            break;
        case State::FSTOP_EXPOSURE_CONFIG:
            if (event == MsgType::BUTTON_MODE_PRESS) {
                status_->setMode(State::FSTOP_EXPOSURE);
            } else if (event == MsgType::MODE_BUTTON_PRESS) {
                status_->setMode(State::TIME_EXPOSURE_CONFIG);
            }
            break;
        case State::FSTOP_EXPOSURE:
            // Automatic transition back handled externally
            break;
        
        case State::TIME_EXPOSURE_CONFIG:
            if (event == MsgType::BUTTON_MODE_PRESS) {
                status_->setMode(State::TIME_EXPOSURE);
            } else if (event == MsgType::MODE_BUTTON_PRESS) {
                status_->setMode(State::FOCUS_LIGHT_OFF);
            }
            break;
        case State::TIME_EXPOSURE:
            // Automatic transition back handled externally
            break;  
    }
}

