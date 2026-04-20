#ifndef EXPOSURE_CALCULATOR_H
#define EXPOSURE_CALCULATOR_H


/**
 * @brief Granularity for exposure adjustments
 */
enum class Granularity 
{
    FullStops, /**< 1 stop increments */
    Halfs,    /**< 1/2 stop increments */
    Thirds,   /**< 1/3 stop increments */
    Sixths,   /**< 1/6 stop increments */
    Twelths   /**< 1/12 stop increments */
};

#define MIN_STEP -3  /**< Minimum step value */
#define MAX_STEP 3   /**< Maximum step value */

/**
 * @brief Class for calculating photographic exposure times
 *
 * This class calculates exposure times based on base time and f-stop values,
 * following the photographic exposure reciprocity law, and also provides
 * calculations for test strip and exposure modes with granularity.
 */
class ExposureCalculator {
public:

    /**
     * @brief Calculate exposure time for test strip mode
     * @param baseTime Base exposure time in seconds
     * @param granularity Granularity of adjustment
     * @param step Step number between MIN_STEP and MAX_STEP
     * @param incrementMode If true, calculate cumulative increment times; if false, calculate individual step times
     * @return Calculated exposure time in seconds
     */
    static double calculateTestStripTime(double baseTime, Granularity granularity, int step, bool incrementMode);

    /**
     * @brief Calculate exposure time for normal exposure mode
     * @param baseTime Base exposure time in seconds
     * @param granularity Granularity of adjustment
     * @param step Step adjustment between MIN_STEP and MAX_STEP
     * @return Calculated exposure time in seconds
     */
    static double calculateExposureTime(double baseTime, Granularity granularity, int step);

private:
    ExposureCalculator();

    /**
     * @brief Get the increment factor for a given granularity
     * @param granularity Granularity setting
     * @return Increment factor (power of 2 per step)
     */
    static double getIncrementFactor(Granularity granularity);
};

#endif