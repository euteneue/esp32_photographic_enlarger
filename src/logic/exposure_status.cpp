#include "exposure_status.h"
#include "config.h"


// Constructors
ExposureStatus::ExposureStatus(double exposureTime, int step, State mode, Granularity granularity, bool iterativeMode) 
    : exposureTime(exposureTime), step(step), mode(mode), granularity(granularity), iterativeMode(iterativeMode) 
    {

    }
    
    
ExposureStatus::ExposureStatus() : exposureTime(MIN_EXPOSURE_TIME), step(0), mode(State::INITIAL), granularity(Granularity::Halfs), iterativeMode(true) 
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

State ExposureStatus::getMode() const 
{
    return mode;
}


Granularity ExposureStatus::getGranularity() const {
    return granularity;
}

bool ExposureStatus::isIterativeMode() const
{
    return iterativeMode;
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

void ExposureStatus::setMode(State mode) 
{
    this->mode = mode;
}

void ExposureStatus::setGranularity(Granularity granularity) 
{
    // Only allow setting granularity if the mode is TestStrip -> once in Exposure mode, the granularity is 
    // fixed to what it was when the mode was switched from TestStrip to Exposure
    if (this->mode == State::TEST_STRIP_CONFIG) 
    {
        this->granularity = granularity;
    }
}


void ExposureStatus::toggleIterativeMode()
{
    // Only allow toggling iterative mode if the mode is TestStrip -> once in Exposure mode, the iterative/single step mode is 
    // fixed to what it was when the mode was switched from TestStrip to Exposure
    if (this->mode == State::TEST_STRIP_CONFIG) 
    {
        this->iterativeMode = !this->iterativeMode;
    }
}
