#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <Preferences.h>
#include <Elog.h>

#include "config.h"
#include "logic/exposure_calculator.h"
#include "logic/fsm.h"
#include "hal/exposure_timer.h"

// Hardware instances
ExposureTimer* timer;

// Logic instances
ExposureCalculator calculator(BASE_TIME_DEFAULT, BASE_FSTOP_DEFAULT);
FSM fsm;

// RTOS task handles
TaskHandle_t inputHandlerTask;
TaskHandle_t displayUpdateTask;
TaskHandle_t exposureTimerTask;
TaskHandle_t stateManagerTask;

// Queues for inter-task communication
QueueHandle_t inputQueue;
QueueHandle_t displayQueue;

// Shared variables
float baseTime = BASE_TIME_DEFAULT;
int baseFStop = BASE_FSTOP_DEFAULT;
int currentFStop = BASE_FSTOP_DEFAULT;
float calculatedTime = BASE_TIME_DEFAULT;

// Function prototypes
void inputHandler(void *pvParameters);
void displayUpdate(void *pvParameters);
void exposureTimer(void *pvParameters);
void stateManager(void *pvParameters);

void setup() 
{
    Serial.begin(115200);
    Logger.registerSerial(MYLOG, ELOG_LEVEL_DEBUG, "DarkroomTimer", Serial);
    Logger.log(MYLOG, ELOG_LEVEL_INFO, "Starting Darkroom Timer...");


    Logger.log(MYLOG, ELOG_LEVEL_INFO, "initializing peripherals...");
    timer = &ExposureTimer::getInstance(new ExposureStatus(),
                              new Relay(RELAY_PIN), 
                              new AiEsp32RotaryEncoder(ENCODER1_DT, ENCODER1_CLK, ENCODER1_SW, ENCODER1_VCC, ENCODER1_STEPS), 
                              new AiEsp32RotaryEncoder(ENCODER2_DT, ENCODER2_CLK, ENCODER2_SW, ENCODER2_VCC, ENCODER2_STEPS), 
                              new TM1638Interface(TM1638_DIO, TM1638_CLK, TM1638_STB));


    Logger.log(MYLOG, ELOG_LEVEL_INFO, "initializing rotary encoders...");                              
    timer->setup();

    Logger.log(MYLOG, ELOG_LEVEL_INFO, "creating queues and tasks...");

    // Create queues
    inputQueue = xQueueCreate(10, sizeof(QueueItem));
    displayQueue = xQueueCreate(10, sizeof(QueueItem));

    // Create tasks
    xTaskCreate(inputHandler, "InputHandler", 2048, NULL, 2, &inputHandlerTask);
    xTaskCreate(displayUpdate, "DisplayUpdate", 2048, NULL, 1, &displayUpdateTask);
    xTaskCreate(exposureTimer, "ExposureTimer", 2048, NULL, 3, &exposureTimerTask);
    xTaskCreate(stateManager, "StateManager", 2048, NULL, 1, &stateManagerTask);

    Logger.log(MYLOG, ELOG_LEVEL_INFO, "finished initialization...");
}

void loop() {
    // Main loop can be empty as tasks handle everything
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// Task implementations (stubs)
void inputHandler(void *pvParameters) {
    int lastPos1 = 0, lastPos2 = 0;
    QueueItem msg;

    while (true) 
    {
        // - user has turned the mode encoder
        if (timer->getEncoderMode()->encoderChanged()) 
        {
            timer->getStatus()->toggleMode();
            msg.type = MsgType::ENCODER_MODE_CHANGE;
            msg.payload = nullptr;

            const char* modeName = nullptr;
            switch (timer->getStatus()->getMode()) {
                case Mode::TestStrip:
                    modeName = "TestStrip";
                    break;
                case Mode::Exposure:
                    modeName = "Exposure";
                    break;
                case Mode::FocusLight:
                    modeName = "FocusLight";
                    break;
            }

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has turned the mode encoder, new mode: %s", modeName);

            xQueueSend(inputQueue, &msg, 0);
        }

        // - user has turned the value encoder
        if (timer->getEncoderValue()->encoderChanged()) 
        {
            int value = timer->getEncoderValue()->readEncoder();

            msg.type = MsgType::ENCODER_VALUE_CHANGE;
            msg.payload = &value;

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has turned the value encoder, new value: %d", value);

            xQueueSend(inputQueue, &msg, 0);            
        }

        if (timer->getEncoderMode()->isEncoderButtonClicked()) 
        {
            msg.type = MsgType::BUTTON_MODE_PRESS;
            msg.payload = nullptr;

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has pressed the mode encoder button");

            xQueueSend(inputQueue, &msg, 0);
        }

        if (timer->getEncoderValue()->isEncoderButtonClicked()) 
        {
            msg.type = MsgType::BUTTON_VALUE_PRESS;
            msg.payload = nullptr;

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has pressed the value encoder button");

            xQueueSend(inputQueue, &msg, 0);
        }
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void displayUpdate(void *pvParameters) {
    QueueItem *msg = new QueueItem;

    while (true) 
    {
        if (xQueueReceive(displayQueue, msg, portMAX_DELAY)) 
        {
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "received message to update display");
            switch (msg->type) {
                case MsgType::ENCODER_MODE_CHANGE:
                    timer->getDisplay()->displayMode(*timer->getStatus());
                    Logger.log(MYLOG, ELOG_LEVEL_INFO, "display updated to show new mode");
                    break;
                case MsgType::ENCODER_VALUE_CHANGE:
                    if (timer->getStatus()->getMode() == Mode::TestStrip) {
                        timer->getDisplay()->displayTimeandStep(*timer->getStatus());
                        Logger.log(MYLOG, ELOG_LEVEL_INFO, "display updated to show new time and step");
                    } else {
                        timer->getDisplay()->displayTime(*timer->getStatus());
                        Logger.log(MYLOG, ELOG_LEVEL_INFO, "display updated to show new time");
                    }
                    break;
            }
        }
        /*
        State state = fsm.getCurrentState();
        char buffer[9];
        switch (state) {
            case SET_BASE_TIME:
                sprintf(buffer, "BT%4.1f", baseTime);
                break;
            case SET_F_STOP:
                sprintf(buffer, "FS%2d", currentFStop);
                break;
            case CALCULATE_TIME:
                sprintf(buffer, "CT%4.1f", calculatedTime);
                break;
            case EXPOSE:
                sprintf(buffer, "EX%4.1f", calculatedTime);
                break;
        }
        //tm1638.setDisplay(buffer);
        */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void exposureTimer(void *pvParameters) 
{
    while (true) 
    {
        /*
        if (fsm.getCurrentState() == EXPOSE) {
            relay.on();
            vTaskDelay(pdMS_TO_TICKS(calculatedTime * 1000));
            relay.off();
            // After exposure, go back to calculate
            fsm.setState(CALCULATE_TIME);
        }
            */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void stateManager(void *pvParameters) 
{
    QueueItem *input = new QueueItem;

    while (true) 
    {
        if (xQueueReceive(inputQueue, input, portMAX_DELAY)) 
        {
            switch (input->type) {
                case MsgType::BUTTON_VALUE_PRESS:
                    fsm.processInput(0); // next state
                    Logger.log(MYLOG, ELOG_LEVEL_INFO, "button value pressed, transitioning to next state");
                    break;
                case MsgType::BUTTON_MODE_PRESS:
                    fsm.processInput(0); // next state
                    Logger.log(MYLOG, ELOG_LEVEL_INFO, "button mode pressed, transitioning to next state");
                    break;
                case MsgType::ENCODER_VALUE_CHANGE:
                    fsm.processInput(1); // adjust value up
                    Logger.log(MYLOG, ELOG_LEVEL_INFO, "encoder value changed, adjusting value up");
                    break;
                case MsgType::ENCODER_MODE_CHANGE:
                    //fsm.processInput(-1); // adjust value down
                    xQueueSend(displayQueue, input, 0); // send to display task to update mode
                    Logger.log(MYLOG, ELOG_LEVEL_INFO, "encoder mode changed, adjusting value down");
                    break;
            }


            /*
            State state = fsm.getCurrentState();
            if (input == 0) { // button press
                fsm.processInput(0); // next state
            } else if (input == 1 || input == -1) {
                // adjust values
                switch (state) {
                    case SET_BASE_TIME:
                        baseTime += input * 0.1f;
                        if (baseTime < 0.1f) baseTime = 0.1f;
                        calculator.setBaseTime(baseTime);
                        break;
                    case SET_F_STOP:
                        currentFStop += input;
                        if (currentFStop < 1) currentFStop = 1;
                        if (currentFStop > 22) currentFStop = 22;
                        break;
                    case CALCULATE_TIME:
                        calculatedTime = calculator.calculateTime(currentFStop);
                        break;
                }
            }
            */
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}