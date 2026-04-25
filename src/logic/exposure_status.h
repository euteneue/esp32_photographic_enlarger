#ifndef EXPOSURE_STATUS_H
#define EXPOSURE_STATUS_H

#include "exposure_calculator.h"
#include "config.h"


/**
 * @brief Class representing the current exposure status and settings
 *
 * This class encapsulates all the state information for photographic enlarger
 * exposure calculations, including time, step adjustments, mode, and granularity.
 */
class ExposureStatus
{
private:
    double exposureTime;  /**< Current exposure time in seconds */
    int step;             /**< Current step adjustment (-3 to +3) */
    State mode;            /**< Current operating mode */
    Granularity granularity; /**< Current adjustment granularity */
    bool iterativeMode; /**< Whether test strip sequence is in iterative mode (true) or single step mode (false) */

public:
    /**
     * @brief Construct a new ExposureStatus object with specified parameters
     * @param exposureTime Initial exposure time in seconds
     * @param step Initial step adjustment
     * @param mode Initial operating mode
     * @param granularity Initial adjustment granularity
     * @param iterativeMode Initial test strip sequence mode (iterative or single step)
     */
    ExposureStatus(double exposureTime, int step, State mode, Granularity granularity, bool iterativeMode);

    /**
     * @brief Construct a new ExposureStatus object with default values
     */
    ExposureStatus();

    /**
     * @brief Get the current exposure time
     * @return Exposure time in seconds
     */
    double getExposureTime() const;

    /**
     * @brief Get the current step adjustment
     * @return Step adjustment value
     */
    int getStep() const;

    /**
     * @brief Get the current operating mode
     * @return Current mode (TestStrip, Exposure or FocusLight)
     */
    State getMode() const;

    /**
     * @brief Get the current adjustment granularity
     * @return Current granularity setting
     */
    Granularity getGranularity() const;

    /**
     * @brief Get whether the test strip sequence is in iterative mode (true) or single step mode (false)
     * 
     * @return true When the iterative mode is active, meaning that each step in the test strip sequence will be an increment from the previous step, rather than being calculated from the base time. In iterative mode, the exposure time for each step is calculated based on the previous step's time, allowing for a more gradual progression through the steps.
     * @return false When the single step mode is active, meaning that each step in the test strip sequence is calculated independently from the base time, using the same base time for all steps. In single step mode, the exposure time for each step is calculated directly from the base time and the step adjustment, without reference to the previous step's time.
     */
    bool isIterativeMode() const; 

    /**
     * @brief Toggle between TestStrip and Exposure modes
     */
    //void toggleMode();

    /**
     * @brief Set the exposure time
     * @param exposureTime New exposure time in seconds
     */
    void setExposureTime(double exposureTime);

    /**
     * @brief Set the step adjustment
     * @param step New step adjustment value
     */
    void setStep(int step);

    /**
     * @brief Set the operating mode
     * @param mode New operating mode
     */
    void setMode(State mode);

    /**
     * @brief Set the adjustment granularity
     * @param granularity New granularity setting
     */
    void setGranularity(Granularity granularity);

    /**
     * @brief Toggle between iterative and single step mode for test strip sequence
     * 
     */
    void toggleIterativeMode();
};

#endif // EXPOSURE_STATUS_H