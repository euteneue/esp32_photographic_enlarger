#include "exposure_status.h"

// Constructors
ExposureStatus::ExposureStatus(double exposureTime, int step, Mode mode, Granularity granularity) 
    : exposureTime(exposureTime), step(step), mode(mode), granularity(granularity) 
    {

    }
    
    
ExposureStatus::ExposureStatus() : exposureTime(MIN_EXPOSURE_TIME), step(0), mode(Mode::TestStrip), granularity(Granularity::Halfs) 
    {

    }

// Getters

double ExposureStatus::getExposureTime() const 
{
    return exposureTime;
}

int ExposureStatus::getStep() const 
{
    return step;
}

Mode ExposureStatus::getMode() const 
{
    return mode;
}

void ExposureStatus::toggleMode() 
{
    switch (mode) {
        case Mode::TestStrip:
            mode = Mode::Exposure;
            break;
        case Mode::Exposure:
            mode = Mode::FocusLight;
            break;
        case Mode::FocusLight:
            mode = Mode::TestStrip;
            break;
    }
}

Granularity ExposureStatus::getGranularity() const {
    return granularity;
}   

// Setters

void ExposureStatus::setExposureTime(double exposureTime) 
{
    if (exposureTime >= MIN_EXPOSURE_TIME && exposureTime <= MAX_EXPOSURE_TIME) 
    {
        this->exposureTime = exposureTime;
    }
}   

void ExposureStatus::setStep(int step) 
{
    if (step >= MIN_STEP && step <= MAX_STEP) 
    {
        this->step = step;
    }
}

void ExposureStatus::setMode(Mode mode) 
{
    this->mode = mode;
}

void ExposureStatus::setGranularity(Granularity granularity) 
{
    // Only allow setting granularity if the mode is TestStrip -> once in Exposure mode, the granularity is 
    // fixed to what it was when the mode was switched from TestStrip to Exposure
    if (this->mode == Mode::TestStrip) 
    {
        this->granularity = granularity;
    }
}
