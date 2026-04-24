#include "config.h"
#include "tm1638_interface.h"
#include <ELog.h>

TM1638Interface::TM1638Interface(int dio, int clk, int stb, SemaphoreHandle_t mutex) : tm1638_(stb, clk, dio, true), tm1638Mutex_(mutex), lastButtonsState_(0), lastButtonReadTime_(0), currentButtonsState_(0) {
    
    tm1638_.displayBegin();
    clear();
}

void TM1638Interface::clear() 
{
    if (xSemaphoreTake(tm1638Mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
        tm1638_.setLEDs(0);
        tm1638_.displayText(" ");
        xSemaphoreGive(tm1638Mutex_);
        Logger.log(MYLOG, ELOG_LEVEL_INFO, "cleared tm1638 display");
    } else {
        Logger.log(MYLOG, ELOG_LEVEL_ERROR, "failed to acquire mutex to clear tm1638 display");
    }
}

void TM1638Interface::setDisplay(const char* str) 
{
    if (xSemaphoreTake(tm1638Mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
        tm1638_.displayText(str);
        xSemaphoreGive(tm1638Mutex_);
        //Logger.log(MYLOG, ELOG_LEVEL_INFO, "updated tm1638 display to \"%s\"", str);
    } else {
        Logger.log(MYLOG, ELOG_LEVEL_ERROR, "failed to acquire mutex to update tm1638 display");
    }
}

void TM1638Interface::setLEDs(uint8_t leds) 
{
    if (xSemaphoreTake(tm1638Mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
        tm1638_.setLEDs(leds);
        xSemaphoreGive(tm1638Mutex_);
        //Logger.log(MYLOG, ELOG_LEVEL_INFO, "updated tm1638 LEDs to 0x%02X", leds);
    } else {
        Logger.log(MYLOG, ELOG_LEVEL_ERROR, "failed to acquire mutex to update tm1638 LEDs");
    }
}

void TM1638Interface::setBrightness(uint8_t brightness) 
{
    tm1638_.brightness(brightness);
}

/*
void TM1638Interface::displayTimeandStep(ExposureStatus status) 
{
    // Display the exposure time on the first 4 digits, and the step on the last 4 digits
    char displayStr[11]; // 8 characters + null terminator
    snprintf(displayStr, sizeof(displayStr), " %4.1f %+2d", status.getExposureTime(), status.getStep());
    setDisplay(displayStr);

    Logger.log(MYLOG, ELOG_LEVEL_INFO, "displayed \"%s\"", displayStr);
}

void TM1638Interface::displayTime(ExposureStatus status) 
{
    // Display the exposure time on the first 4 digits, and blank the last 4 digits
    char displayStr[12]; // 8 characters + null terminator
    snprintf(displayStr, sizeof(displayStr), "time: %4.1f ", status.getExposureTime());
    setDisplay(displayStr);

    Logger.log(MYLOG, ELOG_LEVEL_INFO, "displayed \"%s\"", displayStr);
}

void TM1638Interface::displayStep(ExposureStatus status) 
{
    // Display the step on the last 4 digits, and blank the first 4 digits
    char displayStr[12]; // 8 characters + null terminator
    snprintf(displayStr, sizeof(displayStr), "step: %+2d ", status.getStep());
    setDisplay(displayStr);
    
    Logger.log(MYLOG, ELOG_LEVEL_INFO, "displayed \"%s\"", displayStr);
}

void TM1638Interface::displayMode(ExposureStatus status) 
{
    // Display the current mode (TestStrip, Exposure or FocusLight) on the display
    
    switch (status.getMode()) {
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

    if (strlen(displayBuffer_) > 0) {
        setDisplay(displayBuffer_);
        //Logger.log(MYLOG, ELOG_LEVEL_INFO, "displayed \"%s\"", displayBuffer_);        
    }
}
*/
uint8_t TM1638Interface::getButtons() 
{   
    
    uint8_t buttons;
    uint8_t debouncedButtons = 0;

    if (xSemaphoreTake(tm1638Mutex_, pdMS_TO_TICKS(100)) == pdTRUE) {
        buttons = tm1638_.readButtons();
        xSemaphoreGive(tm1638Mutex_);
    } else {
        Logger.log(MYLOG, ELOG_LEVEL_ERROR, "failed to acquire mutex to read tm1638 buttons");
        return 0; // Return 0 if we can't read the buttons
    }

    // The TM1638 library sometimes returns 0xFF when no buttons are pressed, so we need to convert that to 0 for our logic
    if (buttons == 0xFF) 
    {
        // No buttons pressed, return 0
        buttons = 0;
    }   

    if (buttons != lastButtonsState_) {
        lastButtonReadTime_ = millis();
        Logger.log(MYLOG, ELOG_LEVEL_INFO, "change in button state detected: 0x%02X", buttons);
    }

    if ((millis() - lastButtonReadTime_) > 100) { // 100 ms debounce time
        
        if (buttons != currentButtonsState_) {
            currentButtonsState_ = buttons;
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "button state changed to: 0x%02X", buttons);
            debouncedButtons = buttons;
        }
    } 

    lastButtonsState_ = buttons;    
    return debouncedButtons;
}

