#include "exposure_timer.h"
#include <Arduino.h>
#include <ELog.h>

// Initialize static instance
ExposureTimer* ExposureTimer::instance_ = nullptr;

ExposureTimer& ExposureTimer::getInstance(ExposureStatus* status, Relay* relay, 
                                         AiEsp32RotaryEncoder* encoderValue, 
                                         AiEsp32RotaryEncoder* encoderMode, 
                                         TM1638Interface* display,
                                         Beeper* beeper, 
                                         QueueHandle_t inputQueue, 
                                         QueueHandle_t displayQueue,
                                         QueueHandle_t beeperQueue) {
    if (instance_ == nullptr) {
        // Create instance only if it doesn't exist and we have all required parameters
        if (status && relay && encoderValue && encoderMode && display && beeper && inputQueue && displayQueue && beeperQueue) {
            instance_ = new ExposureTimer(status, relay, encoderValue, encoderMode, display, beeper, inputQueue, displayQueue, beeperQueue);
        } else {
            // If instance doesn't exist but parameters are null, this is an error
            // In a real implementation, you might want to handle this differently
            // For now, we'll assume getInstance is called with parameters on first call
        }
    }
    return *instance_;
}


ExposureTimer::ExposureTimer(ExposureStatus* status, Relay* relay, AiEsp32RotaryEncoder* encoderValue, AiEsp32RotaryEncoder* encoderMode, TM1638Interface* display, Beeper* beeper, QueueHandle_t inputQueue, QueueHandle_t displayQueue, QueueHandle_t beeperQueue)
    : status_(status), relay_(relay), encoderValue_(encoderValue), encoderMode_(encoderMode), display_(display), beeper_(beeper), inputQueue_(inputQueue), displayQueue_(displayQueue), beeperQueue_(beeperQueue) ,exposing_(false), remainingTimeMs_(0), lastTickMs_(0), msg_() 
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

    // Set up interrupt service routines for encoders to handle rotation events
    encoderMode_->setup(readEncoderModeISR);
    encoderValue_->setup(readEncoderValueISR);

    // Set boundaries and acceleration for encoders based on the defined constants in config.h   
    // With the "mode" encoder we face the challenge that we need to be able to switch between
    // setting granularity and step, which have different boundaries. For simplicity, we will set 
    // the mode encoder to have a range that covers all possible values for both step and 
    // granularity, and then handle the interpretation of those values in the state manager 
    // based on the current mode.
    encoderMode_->setBoundaries(0, NUM_STEPS*NUM_GRANULARITY_LEVELS, true); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
    encoderValue_->setBoundaries(MIN_EXPOSURE_TIME, MAX_EXPOSURE_TIME, true); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)

    encoderMode_->setAcceleration(0); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration
    encoderValue_->setAcceleration(250); //or set the value - larger number = more accelearation; 0 or 1 means disabled acceleration

    display_->setBrightness(0);
    display_->setLEDState(0); // Start with all LEDs off
    display_->setLEDs();
    displayMode();

    beeper_->doubleBeep();
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

Beeper* ExposureTimer::getBeeper() const
{
    return beeper_;
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

void ExposureTimer::refreshDisplay() 
{
    // This function can be called to update the display based on the current status
    // For example, it could show remaining time during an exposure, or current mode/step when not exposing

    msg_.type = MsgType::MODE_CHANGED;
    msg_.payload = nullptr;
    xQueueSend(displayQueue_, &msg_, 0);
}

void ExposureTimer::displayMode() 
{
    // Display the current mode (TestStrip, Exposure or FocusLight) on the display
    // This method takes over the compositioning of the display buffer for mode display, 
    // and sends a message to the display manager to update the TM1638 display with the 
    // new buffer. The display buffer is prepared based on the current mode.
    
    switch (status_->getMode()) {
        case State::INITIAL:
            snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "WELCOME ");
            break;
        case State::FOCUS_LIGHT_OFF:
            snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "FOCUS OF");
            break;
        case State::FOCUS_LIGHT_ON:
            snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "FOCUS ON");
            break;
        case State::TEST_STRIP_CONFIG:
            snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "STRPCONF");
            break;
        case State::TEST_STRIP_SEQUENCE:
            snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "");
            break;
        case State::FSTOP_EXPOSURE_CONFIG:
            snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "FSTP EXP");
            break;
        case State::FSTOP_EXPOSURE:
            snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "");
            break;
        case State::TIME_EXPOSURE_CONFIG:
            snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "TImE EXP");
            break;
        case State::TIME_EXPOSURE:
            snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "");
            break;
        default:
            snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "");
            break;
    }
}



void ExposureTimer::displayTimeandGranularity() 
{
    char granularityBuffer[5];
    switch (status_->getGranularity()) {
        case Granularity::FullStops:
            snprintf(granularityBuffer, sizeof(granularityBuffer), "   1");
            break;
        case Granularity::Halfs:
            snprintf(granularityBuffer, sizeof(granularityBuffer), " 1/2");
            break;
        case Granularity::Thirds:
            snprintf(granularityBuffer, sizeof(granularityBuffer), " 1/3");
            break;
        case Granularity::Sixths:
            snprintf(granularityBuffer, sizeof(granularityBuffer), " 1/6");
            break;
        case Granularity::Twelths:
            snprintf(granularityBuffer, sizeof(granularityBuffer), "1/12");
            break;
        default:
            snprintf(granularityBuffer, sizeof(granularityBuffer), "   ?");
            break;
    }
    snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, " %4.1f%s", status_->getExposureTime(), granularityBuffer);
    
    Logger.log(MYLOG, ELOG_LEVEL_INFO, "displayed \"%s\"", displayBuffer_);
}

void ExposureTimer::displayTimeandStep() 
{
    snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, " %4.1f  %+2d", status_->getExposureTime(), status_->getStep());
    
    Logger.log(MYLOG, ELOG_LEVEL_INFO, "displayed \"%s\"", displayBuffer_);
}

void ExposureTimer::displayTime() 
{
    // Display the exposure time on the first 4 digits, and blank the last 4 digits

    snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "%4.1f     ", status_->getExposureTime());

    Logger.log(MYLOG, ELOG_LEVEL_INFO, "displayed \"%s\"", displayBuffer_);
}

void ExposureTimer::displayExposingTime(unsigned long timeMs)
{
    double timeSec = timeMs / 1000.0f;

    snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "%4.1f     ", timeSec);

    Logger.log(MYLOG, ELOG_LEVEL_INFO, "displayed \"%s\"", displayBuffer_);    
}

void ExposureTimer::displayStep() 
{
    // Display the step on the last 4 digits, and blank the first 4 digits

    snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "step: %+2d ", status_->getStep());
        
    Logger.log(MYLOG, ELOG_LEVEL_INFO, "displayed \"%s\"", displayBuffer_);
}

void ExposureTimer::displayMessage(const char *message)
{
    snprintf(displayBuffer_, MAX_DISPLAY_STR_LEN, "%s", message);
        
    Logger.log(MYLOG, ELOG_LEVEL_INFO, "displayed \"%s\"", displayBuffer_);    
}

/**
 * @brief Process input events and update the exposure timer state accordingly. This method implements
 * the state machine logic for handling user inputs based on the current mode of operation. It takes an event 
 * type and an optional payload (for example, encoder values) and updates the ExposureStatus and display buffer 
 * as needed, then sends a message to the display manager to refresh the TM1638 display with the new information.
 * 
 * @param event The input event to process
 * @param payload Optional pointer to additional data (for example encoder value) associated with the event
 */
void ExposureTimer::processInput(MsgType event, void *payload) 
{
    switch (status_->getMode()) {
        case State::INITIAL:
            if (event == MsgType::MODE_BUTTON_PRESS) {
                status_->setMode(State::FOCUS_LIGHT_OFF);
                displayMode();
            }
            break;
        case State::FOCUS_LIGHT_OFF:
            if (event == MsgType::BUTTON_MODE_PRESS) {
                status_->setMode(State::FOCUS_LIGHT_ON);
                displayMode();
            } else if (event == MsgType::MODE_BUTTON_PRESS) {
                status_->setMode(State::TEST_STRIP_CONFIG);
                displayMode();
            }
            break;
        case State::FOCUS_LIGHT_ON:
            if (event == MsgType::BUTTON_MODE_PRESS) {
                status_->setMode(State::FOCUS_LIGHT_OFF);
                displayMode();
            }
            break;
        case State::TEST_STRIP_CONFIG:
            if (event == MsgType::BUTTON_MODE_PRESS) {
                status_->setMode(State::TEST_STRIP_SEQUENCE);
                status_->setStep(MIN_STEP);
            } else if (event == MsgType::MODE_BUTTON_PRESS) {
                status_->setMode(State::FSTOP_EXPOSURE_CONFIG);
                displayMode();
            } else if (event == MsgType::ENCODER_VALUE_CHANGE) {
                status_->setExposureTime(*((long*) payload) * 0.1f); // Assuming encoder steps of 0.1s
                display_->clear();
                displayTimeandGranularity();
            } else if (event == MsgType::ENCODER_MODE_CHANGE) {
                long modeValue = *(long *) payload;

                modeValue = modeValue % NUM_GRANULARITY_LEVELS; // Wrap around to stay within 0-4
                status_->setGranularity(static_cast<Granularity>(modeValue));
                display_->clear();
                displayTimeandGranularity();
            } else if (event == MsgType::ITERATIVE_BUTTON_PRESS) {
                status_->toggleIterativeMode();
                display_->clear();

                status_->isIterativeMode() ? displayMessage("ITER ON ") : displayMessage("ITER OFF"); 
            }
            break;
        case State::TEST_STRIP_SEQUENCE:
            // Events handled externally for sequence advancement, except for cancel button which can be used to exit the sequence
             if (event == MsgType::CANCEL_BUTTON_PRESS) {
                status_->setMode(State::TEST_STRIP_CONFIG);
                displayMode();
            } 
            break;
        case State::FSTOP_EXPOSURE_CONFIG:
            if (event == MsgType::BUTTON_MODE_PRESS) {
                status_->setMode(State::FSTOP_EXPOSURE);
            } else if (event == MsgType::MODE_BUTTON_PRESS) {
                status_->setMode(State::TIME_EXPOSURE_CONFIG);
                displayMode();
            } else if (event == MsgType::ENCODER_VALUE_CHANGE) {
                status_->setExposureTime(*((long*) payload) * 0.1f); // Assuming encoder steps of 0.1s
                display_->clear();
                displayTimeandStep();
            } else if (event == MsgType::ENCODER_MODE_CHANGE) {
                long modeValue = *(long *) payload;
                int stepValue = (modeValue % NUM_STEPS) + MIN_STEP; // Wrap around to stay within -3 to +3

                status_->setStep(stepValue);
                display_->clear();
                displayTimeandStep();
            }
            break;
        case State::FSTOP_EXPOSURE:
            // Automatic transition back handled externally
             if (event == MsgType::CANCEL_BUTTON_PRESS) {
                status_->setMode(State::FSTOP_EXPOSURE_CONFIG);
                displayMode();
            }            
            break;
        
        case State::TIME_EXPOSURE_CONFIG:
            if (event == MsgType::BUTTON_MODE_PRESS) {
                status_->setMode(State::TIME_EXPOSURE);
            } else if (event == MsgType::MODE_BUTTON_PRESS) {
                status_->setMode(State::FOCUS_LIGHT_OFF);
                displayMode();
            } else if (event == MsgType::ENCODER_VALUE_CHANGE) {
                status_->setExposureTime(*((long*) payload) * 0.1f); // Assuming encoder steps of 0.1s
                display_->clear();
                displayTime();
            }
            break;
        case State::TIME_EXPOSURE:
            // Automatic transition back handled externally
             if (event == MsgType::CANCEL_BUTTON_PRESS) {
                status_->setMode(State::TIME_EXPOSURE_CONFIG);
                displayMode();
            }            
            break;  
    }
}

void ExposureTimer::countdownExposureTime(double exposureTime)
{
    double elapsedTime = 0;
    QueueItem msg;

    while (elapsedTime < (exposureTime * 1000))
    {
        // Check if we received a cancel message
        if ((getStatus()->getMode() != TIME_EXPOSURE) && (getStatus()->getMode() != FSTOP_EXPOSURE) && (getStatus()->getMode() != State::TEST_STRIP_SEQUENCE))
        {
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "Exposure cancelled after %.2f seconds", elapsedTime / 1000.0f);
            break;
        }

        // Every second, send a tick message to the beeper to provide audio feedback during the 
        // exposure. As beeping blocks the execution of the host task for a short time, we will
        // use a non-blocking approach to trigger the beeper every second without blocking the 
        // exposure timer task. This allows us to maintain accurate timing for the exposure while 
        // still providing regular audio feedback.
        if ((long) ((exposureTime*1000) - elapsedTime) % 1000 < 90) 
        {
            msg.type = MsgType::BEEPER_TICK;
            msg.payload = nullptr;

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "Exposure tick at %.2f seconds remaining", (exposureTime*1000 - elapsedTime) / 1000.0f);
            xQueueSend(beeperQueue_, &msg, 0);
        }
        displayExposingTime((exposureTime * 1000) - elapsedTime);
        vTaskDelay(pdMS_TO_TICKS(100));
        elapsedTime += 100;
    }

    // After the exposure is complete (either because the time has elapsed or because it was cancelled),
    // we can trigger a double beep to signal the end of the exposure. This provides clear audio feedback
    // to the user that the exposure has finished.
    msg.type = MsgType::BEEPER_DOUBLE_BEEP;
    msg.payload = nullptr;

    xQueueSend(beeperQueue_, &msg, 0);
}

void ExposureTimer::exposureControl()
{
        State currentState = getStatus()->getMode();

        if (currentState == FSTOP_EXPOSURE) {
            double exposureTime = ExposureCalculator::calculateExposureTime(
                getStatus()->getExposureTime(),
                getStatus()->getGranularity(),
                getStatus()->getStep()
            );
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "Starting F-Stop exposure for %.2f seconds", exposureTime);
            getRelay()->on();

            // Instead of using vTaskDelay, we will use a loop to check for cancellation every 100ms
            countdownExposureTime(exposureTime);

            getRelay()->off();
            getStatus()->setMode(State::FSTOP_EXPOSURE_CONFIG);
            displayTimeandStep();
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "F-Stop exposure completed");
        } else if (currentState == TIME_EXPOSURE) {
            double exposureTime = getStatus()->getExposureTime();

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "Starting Time-based exposure for %.2f seconds", exposureTime);
            getRelay()->on();

            // Instead of using vTaskDelay, we will use a loop to check for cancellation every 100ms
            countdownExposureTime(exposureTime);

            getRelay()->off();
            getStatus()->setMode(State::TIME_EXPOSURE_CONFIG);     
            displayTime();       
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "Time-based exposure completed");
        } else if (currentState == State::TEST_STRIP_SEQUENCE) {
            double exposureTime = ExposureCalculator::calculateTestStripTime(
                getStatus()->getExposureTime(),
                getStatus()->getGranularity(),
                getStatus()->getStep(),
                getStatus()->isIterativeMode()
            );
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "Starting test strip exposure step %d for %.2f seconds, iter: %d", getStatus()->getStep(), exposureTime, getStatus()->isIterativeMode());
            getDisplay()->setLEDState(1 << (getStatus()->getStep()-MIN_STEP)); // Light up the LED corresponding to the current step
            
            getRelay()->on();
            
            // Instead of using vTaskDelay, we will use a loop to check for cancellation every 100ms
            countdownExposureTime(exposureTime);

            getRelay()->off();
            getStatus()->setMode(State::TEST_STRIP_SEQUENCE);

            if (getStatus()->getStep() < MAX_STEP)
            {
                getStatus()->setStep(getStatus()->getStep() + 1); // Advance to the next step for the next exposure
                displayExposingTime(0); // Clear the display after each step, ready for the next step to update it with the new time
                Logger.log(MYLOG, ELOG_LEVEL_INFO, "Test strip exposure step %d completed, preparing for next step", getStatus()->getStep());
                vTaskDelay(pdMS_TO_TICKS(WAIT_BETWEEN_TEST_STRIP_STEPS_MS));
            } else {
                getDisplay()->setLEDState(0); // Turn off all LEDs after the last step
                getStatus()->setMode(State::TEST_STRIP_CONFIG); // After the last step, return to config mode
                displayTimeandGranularity(); // Display the base time and granularity again in config mode
                Logger.log(MYLOG, ELOG_LEVEL_INFO, "Test strip exposure sequence completed");
            }
        } else if (currentState == State::FOCUS_LIGHT_ON) {
            // Focus light is on, relay should be on
            getRelay()->on();
        } else {
            // For other states, ensure relay is off
            getRelay()->off();
        }    
}

void ExposureTimer::handleInput()
{
      uint8_t tmButtons = getDisplay()->getButtons();

        if (tmButtons & MODE_BUTTON) // Mode button has been pressed
        {
            msg_.type = MsgType::MODE_BUTTON_PRESS;
            msg_.payload = nullptr;

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has pressed the mode button");

            xQueueSend(inputQueue_, &msg_, 0);
        }

        if (tmButtons & CANCEL_BUTTON) // Cancel button has been pressed
        {
            msg_.type = MsgType::CANCEL_BUTTON_PRESS;
            msg_.payload = nullptr;

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has pressed the cancel button");

            xQueueSend(inputQueue_, &msg_, 0);
        }

        if (tmButtons & ITERATIVE_BUTTON) // Iterative button has been pressed
        {
            msg_.type = MsgType::ITERATIVE_BUTTON_PRESS;
            msg_.payload = nullptr;

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has pressed the iterative button");

            xQueueSend(inputQueue_, &msg_, 0);
        }        

        // - user has turned the mode encoder
        if (getEncoderMode()->encoderChanged()) 
        {
            valueBuffer_ = getEncoderMode()->readEncoder();

            msg_.type = MsgType::ENCODER_MODE_CHANGE;
            msg_.payload = &valueBuffer_;
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has turned the value encoder, new value: %d", valueBuffer_);

            xQueueSend(inputQueue_, &msg_, 0);
        }

        // - user has turned the value encoder
        if (getEncoderValue()->encoderChanged()) 
        {
            valueBuffer_ = getEncoderValue()->readEncoder();

            msg_.type = MsgType::ENCODER_VALUE_CHANGE;
            msg_.payload = &valueBuffer_;

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has turned the value encoder, new value: %d", valueBuffer_);

            xQueueSend(inputQueue_, &msg_, 0);            
        }

        if (getEncoderMode()->isEncoderButtonClicked()) 
        {            
            msg_.type = MsgType::BUTTON_MODE_PRESS;
            msg_.payload = nullptr;

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has pressed the mode encoder button");

            xQueueSend(inputQueue_, &msg_, 0);
        }

        if (getEncoderValue()->isEncoderButtonClicked()) 
        {
            msg_.type = MsgType::BUTTON_VALUE_PRESS;
            msg_.payload = nullptr;

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has pressed the value encoder button");

            xQueueSend(inputQueue_, &msg_, 0);
        }    
}

void ExposureTimer::handleBeeper(QueueItem *msg)
{
    switch (msg->type) 
    {
        case MsgType::BEEPER_HIGH:
            getBeeper()->highBeep();
            break;
        case MsgType::BEEPER_TICK:
            getBeeper()->tick();
            break;
        case MsgType::BEEPER_LOW:
            getBeeper()->lowBeep();
            break;
        case MsgType::BEEPER_DOUBLE_BEEP:
            getBeeper()->doubleBeep();
            break;
        default:
            // For other message types, we can choose to do nothing or add more cases as needed
            break;
    }    
}

const char *ExposureTimer::getDisplayBuffer() const
{
    return displayBuffer_;
}
