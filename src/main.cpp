#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <Preferences.h>
#include <Elog.h>

#include "config.h"
#include "logic/exposure_calculator.h"
//#include "logic/fsm.h"
#include "hal/exposure_timer.h"

// Hardware instances
ExposureTimer* timer;

// Logic instances
//FSM fsm;

// RTOS task handles
TaskHandle_t inputHandlerTask;
TaskHandle_t displayUpdateTask;
TaskHandle_t exposureTimerTask;
TaskHandle_t stateManagerTask;

// Queues for inter-task communication
QueueHandle_t inputQueue;
QueueHandle_t displayQueue;

SemaphoreHandle_t tm1638Mutex;

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


void testExposureCalculator() 
{
    double baseTime = 10.0; // seconds

    Serial.printf("Calculating exposure times for base time: %.1f seconds and full stop granularity, single step mode\n", baseTime);
    for (int fstop = MIN_STEP; fstop <= MAX_STEP; fstop++) 
    {
        double time = ExposureCalculator::calculateTestStripTime(baseTime, Granularity::FullStops, fstop, false);
        Serial.printf("BaseTime: %.1f s, Current F-Stop: %d => Calculated Time: %.1f s\n", baseTime, fstop, time);
    }

    Serial.printf("Calculating exposure times for base time: %.1f seconds and full stop granularity, incremental mode\n", baseTime);
    for (int fstop = MIN_STEP; fstop <= MAX_STEP; fstop++) 
    {
        double time = ExposureCalculator::calculateTestStripTime(baseTime, Granularity::FullStops, fstop, true);
        Serial.printf("BaseTime: %.1f s, Current F-Stop: %d => Calculated Time: %.1f s\n", baseTime, fstop, time);
    }

    Serial.printf("Calculating exposure times for base time: %.1f seconds and half stop granularity, single step mode\n", baseTime);
    for (int fstop = MIN_STEP; fstop <= MAX_STEP; fstop++) 
    {
        double time = ExposureCalculator::calculateTestStripTime(baseTime, Granularity::Halfs, fstop, false);
        Serial.printf("BaseTime: %.1f s, Current F-Stop: %d => Calculated Time: %.1f s\n", baseTime, fstop, time);
    }

    Serial.printf("Calculating exposure times for base time: %.1f seconds and half stop granularity, incremental mode\n", baseTime);
    for (int fstop = MIN_STEP; fstop <= MAX_STEP; fstop++) 
    {
        double time = ExposureCalculator::calculateTestStripTime(baseTime, Granularity::Halfs, fstop, true);
        Serial.printf("BaseTime: %.1f s, Current F-Stop: %d => Calculated Time: %.1f s\n", baseTime, fstop, time);
    }    

    Serial.printf("Calculating exposure times for base time: %.1f seconds and third stop granularity, single step mode\n", baseTime);
    for (int fstop = MIN_STEP; fstop <= MAX_STEP; fstop++) 
    {
        double time = ExposureCalculator::calculateTestStripTime(baseTime, Granularity::Thirds, fstop, false);
        Serial.printf("BaseTime: %.1f s, Current F-Stop: %d => Calculated Time: %.1f s\n", baseTime, fstop, time);
    }

    Serial.printf("Calculating exposure times for base time: %.1f seconds and third stop granularity, incremental mode\n", baseTime);
    for (int fstop = MIN_STEP; fstop <= MAX_STEP; fstop++) 
    {
        double time = ExposureCalculator::calculateTestStripTime(baseTime, Granularity::Thirds, fstop, true);
        Serial.printf("BaseTime: %.1f s, Current F-Stop: %d => Calculated Time: %.1f s\n", baseTime, fstop, time);
    }    
}


void setup() 
{
    QueueItem *msg = new QueueItem();

    Serial.begin(115200);
    Logger.registerSerial(MYLOG, ELOG_LEVEL_DEBUG, "DarkroomTimer", Serial);
    Logger.log(MYLOG, ELOG_LEVEL_INFO, "Starting Darkroom Timer...");

    Logger.log(MYLOG, ELOG_LEVEL_INFO, "creating queues and tasks...");

    // Create queues
    inputQueue = xQueueCreate(10, sizeof(QueueItem));
    displayQueue = xQueueCreate(10, sizeof(QueueItem));

    // Create mutex to serialize TM1638 display / button access. This is necessary because the display and 
    // button handling code in the TM1638 library is not thread safe, and we need to ensure that only one 
    // task is accessing the display/buttons at a time to prevent race conditions and potential crashes.
    tm1638Mutex = xSemaphoreCreateMutex();

    Logger.log(MYLOG, ELOG_LEVEL_INFO, "initializing peripherals...");
    timer = &ExposureTimer::getInstance(new ExposureStatus(),
                              new Relay(RELAY_PIN), 
                              new AiEsp32RotaryEncoder(ENCODER1_DT, ENCODER1_CLK, ENCODER1_SW, ENCODER1_VCC, ENCODER1_STEPS), 
                              new AiEsp32RotaryEncoder(ENCODER2_DT, ENCODER2_CLK, ENCODER2_SW, ENCODER2_VCC, ENCODER2_STEPS), 
                              new TM1638Interface(TM1638_DIO, TM1638_CLK, TM1638_STB, tm1638Mutex),
                              inputQueue,
                              displayQueue);


    Logger.log(MYLOG, ELOG_LEVEL_INFO, "initializing rotary encoders...");                              
    timer->setup();    

    // Create tasks
    xTaskCreate(inputHandler, "InputHandler", 2048, NULL, 2, &inputHandlerTask);
    xTaskCreate(displayUpdate, "DisplayUpdate", 2048, NULL, 3, &displayUpdateTask);
    xTaskCreate(exposureTimer, "ExposureTimer", 2048, NULL, 1, &exposureTimerTask);
    xTaskCreate(stateManager, "StateManager", 2048, NULL, 1, &stateManagerTask);

    Logger.log(MYLOG, ELOG_LEVEL_INFO, "finished initialization...");

     //testExposureCalculator();
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
        uint8_t tmButtons = timer->getDisplay()->getButtons();

        if (tmButtons & MODE_BUTTON) // Mode button is the first bit
        {
            msg.type = MsgType::MODE_BUTTON_PRESS;
            msg.payload = nullptr;

            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has pressed the mode button");

            xQueueSend(inputQueue, &msg, 0);
        }
        // - user has turned the mode encoder
        if (timer->getEncoderMode()->encoderChanged()) 
        {
            int value = timer->getEncoderMode()->readEncoder();

            msg.type = MsgType::ENCODER_MODE_CHANGE;
            msg.payload = &value;
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "user has turned the value encoder, new value: %d", value);

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
        timer->getDisplay()->setDisplay(timer->getDisplayBuffer());

        /*
        if (xQueueReceive(displayQueue, msg, portMAX_DELAY)) 
        {
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "received message to update display");

            timer->getDisplay()->displayMode(*timer->getStatus());
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "display updated to show new mode %d", timer->getStatus()->getMode());

            
            switch (msg->type) {
                case MsgType::MODE_CHANGED:
                    timer->getDisplay()->displayMode(*timer->getStatus());
                    Logger.log(MYLOG, ELOG_LEVEL_INFO, "display updated to show new mode %d", timer->getStatus()->getMode());
                    break;                
                case MsgType::MODE_BUTTON_PRESS:
                    timer->getDisplay()->displayMode(*timer->getStatus());
                    Logger.log(MYLOG, ELOG_LEVEL_INFO, "display updated to show new mode %d", timer->getStatus()->getMode());
                    break;
                case MsgType::ENCODER_VALUE_CHANGE:
                    if (timer->getStatus()->getMode() == State::TEST_STRIP_CONFIG) {
                        timer->getDisplay()->displayTimeandStep(*timer->getStatus());
                        Logger.log(MYLOG, ELOG_LEVEL_INFO, "display updated to show new time and step");
                    } else {
                        timer->getDisplay()->displayTime(*timer->getStatus());
                        Logger.log(MYLOG, ELOG_LEVEL_INFO, "display updated to show new time");
                    }
                    break;
            }
            
        }
        */
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void exposureTimer(void *pvParameters) 
{
    while (true) 
    {
        State currentState = timer->getStatus()->getMode();

        if (currentState == FSTOP_EXPOSURE) {
            double exposureTime = ExposureCalculator::calculateExposureTime(
                timer->getStatus()->getExposureTime(),
                timer->getStatus()->getGranularity(),
                timer->getStatus()->getStep()
            );
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "Starting F-Stop exposure for %.2f seconds", exposureTime);
            timer->getRelay()->on();
            vTaskDelay(pdMS_TO_TICKS(exposureTime * 1000));
            timer->getRelay()->off();
            timer->getStatus()->setMode(State::FSTOP_EXPOSURE_CONFIG);
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "F-Stop exposure completed");
        } else if (currentState == State::TEST_STRIP_SEQUENCE) {
            double exposureTime = ExposureCalculator::calculateTestStripTime(
                timer->getStatus()->getExposureTime(),
                timer->getStatus()->getGranularity(),
                timer->getStatus()->getStep(),
                false
            );
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "Starting test strip exposure step %d for %.2f seconds", timer->getStatus()->getStep(), exposureTime);
            timer->getRelay()->on();
            vTaskDelay(pdMS_TO_TICKS(exposureTime * 1000));
            timer->getRelay()->off();
            timer->getStatus()->setMode(State::TEST_STRIP_SEQUENCE);
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "Test strip exposure step completed");
        } else if (currentState == State::FOCUS_LIGHT_ON) {
            // Focus light is on, relay should be on
            timer->getRelay()->on();
        } else {
            // For other states, ensure relay is off
            timer->getRelay()->off();
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void stateManager(void *pvParameters) 
{
    QueueItem *input = new QueueItem;
    static int lastValuePos = 0;
    static int lastModePos = 0;

    while (true) 
    {
        if (xQueueReceive(inputQueue, input, portMAX_DELAY)) 
        {
            timer->processInput(input->type, input->payload);
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}