#include "exposure_status.h"
#include "config.h"
#include <Elog.h>

#define STATUS_NAMESPACE "exp_status"


// Constructors
ExposureStatus::ExposureStatus(double exposureTime, int step, State mode, Granularity granularity, bool iterativeMode) 
    : exposureTime(exposureTime), step(step), mode(mode), granularity(granularity), iterativeMode(iterativeMode), preferences() 
    {

    }
    
    
ExposureStatus::ExposureStatus() : exposureTime(MIN_EXPOSURE_TIME), step(0), mode(State::INITIAL), granularity(Granularity::Halfs), iterativeMode(true), preferences() 
    {
        // Load saved settings from preferences if available
        if (preferences.begin(STATUS_NAMESPACE, true)) {
            this->exposureTime = preferences.getFloat("exposureTime", exposureTime);
            this->step = preferences.getInt("step", step);
            this->mode = static_cast<State>(preferences.getInt("mode", static_cast<int>(mode)));
            this->granularity = static_cast<Granularity>(preferences.getInt("granularity", static_cast<int>(granularity)));
            this->iterativeMode = preferences.getBool("iterativeMode", iterativeMode);
            preferences.end();

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "Loaded exposure status from preferences: time=%.2f, step=%d, mode=%d, granularity=%d, iterativeMode=%d", 
                this->exposureTime, this->step, static_cast<int>(this->mode), static_cast<int>(this->granularity), this->iterativeMode);
        }            
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

void ExposureStatus::saveToPreferences()
{
    if (preferences.begin(STATUS_NAMESPACE, false)) {
        preferences.putFloat("exposureTime", this->exposureTime);
        preferences.putInt("step", this->step);
        preferences.putInt("mode", static_cast<int>(this->mode));
        preferences.putInt("granularity", static_cast<int>(this->granularity));
        preferences.putBool("iterativeMode", this->iterativeMode);
        preferences.end();

        Logger.log(MYLOG, ELOG_LEVEL_INFO, "Saved exposure status to preferences: time=%.2f, step=%d, mode=%d, granularity=%d, iterativeMode=%d", 
            this->exposureTime, this->step, static_cast<int>(this->mode), static_cast<int>(this->granularity), this->iterativeMode);
    } else {
        Logger.log(MYLOG, ELOG_LEVEL_ERROR, "Failed to open preferences for writing exposure status");
    }
}
