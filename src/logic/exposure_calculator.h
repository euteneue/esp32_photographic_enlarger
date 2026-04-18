#ifndef EXPOSURE_CALCULATOR_H
#define EXPOSURE_CALCULATOR_H

/**
 * @brief Class for calculating photographic exposure times
 *
 * This class calculates exposure times based on a base time and f-stop values,
 * following the photographic exposure reciprocity law.
 */
class ExposureCalculator {
public:
    /**
     * @brief Construct a new ExposureCalculator object
     * @param baseTime Base exposure time in seconds
     * @param baseFStop Base f-stop value
     */
    ExposureCalculator(float baseTime, int baseFStop);

    /**
     * @brief Calculate exposure time for a given f-stop
     * @param currentFStop Current f-stop value
     * @return Calculated exposure time in seconds
     */
    float calculateTime(int currentFStop) const;

    /**
     * @brief Set the base exposure time
     * @param time Base exposure time in seconds
     */
    void setBaseTime(float time);

    /**
     * @brief Set the base f-stop value
     * @param fstop Base f-stop value
     */
    void setBaseFStop(int fstop);

private:
    float baseTime_;  /**< Base exposure time in seconds */
    int baseFStop_;   /**< Base f-stop value */
};

#endif