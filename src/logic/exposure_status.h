#ifndef EXPOSURE_STATUS_H
#define EXPOSURE_STATUS_H

/**
 * @brief Operating mode for the enlarger
 */
enum class Mode 
{
    TestStrip,  /**< Test strip mode for determining optimal exposure */
    Exposure,    /**< Normal exposure mode */
    FocusLight    /**< Focus light mode  */
};

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

#define MIN_EXPOSURE_TIME 0     /**< Minimum exposure time in seconds */
#define MAX_EXPOSURE_TIME 999   /**< Maximum exposure time in seconds */

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
    Mode mode;            /**< Current operating mode */
    Granularity granularity; /**< Current adjustment granularity */

public:
    /**
     * @brief Construct a new ExposureStatus object with specified parameters
     * @param exposureTime Initial exposure time in seconds
     * @param step Initial step adjustment
     * @param mode Initial operating mode
     * @param granularity Initial adjustment granularity
     */
    ExposureStatus(double exposureTime, int step, Mode mode, Granularity granularity);

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
    Mode getMode() const;

    /**
     * @brief Get the current adjustment granularity
     * @return Current granularity setting
     */
    Granularity getGranularity() const;

    /**
     * @brief Toggle between TestStrip and Exposure modes
     */
    void toggleMode();

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
    void setMode(Mode mode);

    /**
     * @brief Set the adjustment granularity
     * @param granularity New granularity setting
     */
    void setGranularity(Granularity granularity);
};

#endif // EXPOSURE_STATUS_H