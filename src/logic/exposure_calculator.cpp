#include "exposure_calculator.h"
#include <cmath>

ExposureCalculator::ExposureCalculator(float baseTime, int baseFStop) 
    : baseTime_(baseTime), baseFStop_(baseFStop) {}

float ExposureCalculator::calculateTime(int currentFStop) const {
    int diff = currentFStop - baseFStop_;
    return baseTime_ * pow(2.0f, diff);
}

void ExposureCalculator::setBaseTime(float time) {
    baseTime_ = time;
}

void ExposureCalculator::setBaseFStop(int fstop) {
    baseFStop_ = fstop;
}