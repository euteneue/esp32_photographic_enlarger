#include "config.h"
#include "tm1638_interface.h"
#include <ELog.h>

TM1638Interface::TM1638Interface(int dio, int clk, int stb) : tm1638_(stb, clk, dio, true) {
    tm1638_.displayBegin();
    clear();
}

void TM1638Interface::clear() 
{
    tm1638_.setLEDs(0);
    tm1638_.displayText(" ");
    
    Logger.log(MYLOG, ELOG_LEVEL_INFO, "cleared tm1638 display");
}

void TM1638Interface::setDisplay(const char* str) 
{
    tm1638_.displayText(str);
}

void TM1638Interface::setLEDs(uint8_t leds) 
{
    tm1638_.setLEDs(leds);  
}

void TM1638Interface::setBrightness(uint8_t brightness) 
{
    tm1638_.brightness(brightness);
}

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
    const char* modeStr = nullptr;

    switch (status.getMode()) {
        case State::INITIAL:
            modeStr = "WELCOME";
            break;
        case State::TEST_STRIP_CONFIG:
            modeStr = "SEL:TEST";
            break;
        case State::FSTOP_EXPOSURE_CONFIG:
            modeStr = "SEL:EXP ";
            break;
        case State::FOCUS_LIGHT_OFF:
            modeStr = "SEL:FOCU";
            break;
    }

    setDisplay(modeStr);
    
    Logger.log(MYLOG, ELOG_LEVEL_INFO, "displayed \"%s\"", modeStr);
}

uint8_t TM1638Interface::getButtons() 
{   uint8_t buttons = tm1638_.readButtons();

    delay(300); // Add a small delay to debounce the buttons
    //Logger.log(MYLOG, ELOG_LEVEL_INFO, "read buttons: 0x%02X", buttons);
    return buttons;
}