#include "exposure_calculator.h"
#include <cmath>

ExposureCalculator::ExposureCalculator() {
    // Default constructor can be empty since we are using static methods
}

/**
 * @brief Calculate exposure time for test strip mode
 * 
 * @param baseTime Base exposure time in seconds and fractions of a second (e.g. 0.5 for half a second)
 * @param granularity Granularity of adjustment
 * @param step Step number between MIN_STEP and MAX_STEP
 * @param incrementMode If true, calculate cumulative increment times; if false, calculate individual step times
 * @return double Calculated exposure time in seconds 
 */
double ExposureCalculator::calculateTestStripTime(double baseTime, Granularity granularity, int step, bool incrementMode) {
    if (step < MIN_STEP || step > MAX_STEP) {
        return baseTime; // or throw error, but for now return base
    }
    
    double incrementFactor = getIncrementFactor(granularity);
 
    if (incrementMode) {
        // Cumulative increment mode: We start with the minimum time and add increments for each step up to the current step    
        
        if (step == MIN_STEP) 
        {
            return baseTime * pow(2.0, step * incrementFactor);
        } else {
            double previousTime = baseTime * pow(2.0, (step-1) * incrementFactor);
            double currentTime = baseTime * pow(2.0, step * incrementFactor);
            return currentTime - previousTime;
        }

    } else {
        // Single step mode: each step has its own calculated time
        return baseTime * pow(2.0, step * incrementFactor);
    }
}

/**
 * @brief Calculate exposure time for normal exposure mode
 * 
 * @param baseTime Base exposure time in seconds and fractions of a second (e.g. 0.5 for half a second)
 * @param granularity Granularity of adjustment
 * @param step Step adjustment between MIN_STEP and MAX_STEP
 * @return double Calculated exposure time in seconds
 */
double ExposureCalculator::calculateExposureTime(double baseTime, Granularity granularity, int step) {
    if (step < MIN_STEP || step > MAX_STEP) {
        return baseTime;
    }
    
    double incrementFactor = getIncrementFactor(granularity);
    return baseTime * pow(2.0, step * incrementFactor);
}

/**
 * @brief Helper function to get the increment factor for a given granularity
 * 
 * @param granularity The granularity for which to get the increment factor
 * @return double Increment factor for the given granularity (power of 2 per step)
 */
double ExposureCalculator::getIncrementFactor(Granularity granularity) {
    switch (granularity) {
        case Granularity::FullStops: return 1.0;
        case Granularity::Halfs: return 0.5;
        case Granularity::Thirds: return 1.0/3.0;
        case Granularity::Sixths: return 1.0/6.0;
        case Granularity::Twelths: return 1.0/12.0;
        default: return 1.0;
    }
}