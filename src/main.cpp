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

    testExposureCalculator();
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
            msg.type = MsgType::ENCODER_MODE_CHANGE;
            msg.payload = nullptr;

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
                case MsgType::MODE_BUTTON_PRESS:
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

        // Update display based on FSM state
        State state = fsm.getCurrentState();
        char buffer[9];
        switch (state) {
            case INITIAL:
                sprintf(buffer, "WELCOME");
                break;
            case FOCUS_LIGHT_OFF:
                sprintf(buffer, "F-OFF");
                break;
            case FOCUS_LIGHT_ON:
                sprintf(buffer, "F-ON");
                break;
            case TEST_STRIP_CONFIG:
                sprintf(buffer, "TS-CONF");
                break;
            case TEST_STRIP_SEQUENCE:
                sprintf(buffer, "TS-%d", fsm.getTestStripStep());
                break;
            case FSTOP_EXPOSURE_CONFIG:
                sprintf(buffer, "EXP-CONF");
                break;
            case FSTOP_EXPOSURE:
                sprintf(buffer, "EXPOSING");
                break;
        }
        timer->getDisplay()->setDisplay(buffer);
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void exposureTimer(void *pvParameters) 
{
    while (true) 
    {
        State currentState = fsm.getCurrentState();
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
            fsm.setState(FSTOP_EXPOSURE_CONFIG);
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "F-Stop exposure completed");
        } else if (currentState == TEST_STRIP_SEQUENCE) {
            double exposureTime = ExposureCalculator::calculateTestStripTime(
                timer->getStatus()->getExposureTime(),
                timer->getStatus()->getGranularity(),
                fsm.getTestStripStep(),
                false
            );
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "Starting test strip exposure step %d for %.2f seconds", fsm.getTestStripStep(), exposureTime);
            timer->getRelay()->on();
            vTaskDelay(pdMS_TO_TICKS(exposureTime * 1000));
            timer->getRelay()->off();
            fsm.advanceTestStrip();
            Logger.log(MYLOG, ELOG_LEVEL_INFO, "Test strip exposure step completed");
        } else if (currentState == FOCUS_LIGHT_ON) {
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
            switch (input->type) {
                case MsgType::BUTTON_VALUE_PRESS:
                    // Handle value encoder button press if needed
                    Logger.log(MYLOG, ELOG_LEVEL_INFO, "button value pressed");
                    break;
                case MsgType::BUTTON_MODE_PRESS:
                    fsm.processInput(MsgType::BUTTON_MODE_PRESS);
                    Logger.log(MYLOG, ELOG_LEVEL_INFO, "button mode pressed, processing FSM");
                    break;
                case MsgType::ENCODER_VALUE_CHANGE:
                    {
                        int currentPos = *(int*)input->payload;
                        int direction = (currentPos > lastValuePos) ? 1 : ((currentPos < lastValuePos) ? -1 : 0);
                        lastValuePos = currentPos;
                        if (direction != 0) {
                            State currentState = fsm.getCurrentState();
                            if (currentState == TEST_STRIP_CONFIG || currentState == FSTOP_EXPOSURE_CONFIG) {
                                double currentTime = timer->getStatus()->getExposureTime();
                                currentTime += direction * 0.1f;
                                if (currentTime < 0.1f) currentTime = 0.1f;
                                if (currentTime > 999.0f) currentTime = 999.0f;
                                timer->getStatus()->setExposureTime(currentTime);
                                Logger.log(MYLOG, ELOG_LEVEL_INFO, "adjusted base time to %.1f", currentTime);
                            }
                        }
                    }
                    break;
                case MsgType::MODE_BUTTON_PRESS:
                    fsm.processInput(MsgType::MODE_BUTTON_PRESS);
                    Logger.log(MYLOG, ELOG_LEVEL_INFO, "mode button pressed, processing FSM");
                    break;
                case MsgType::ENCODER_MODE_CHANGE:
                    {
                        int currentPos = *(int*)input->payload;
                        int direction = (currentPos > lastModePos) ? 1 : ((currentPos < lastModePos) ? -1 : 0);
                        lastModePos = currentPos;
                        if (direction != 0) {
                            State currentState = fsm.getCurrentState();
                            if (currentState == TEST_STRIP_CONFIG) {
                                // Cycle granularity
                                Granularity currentGran = timer->getStatus()->getGranularity();
                                int granInt = static_cast<int>(currentGran);
                                granInt += direction;
                                if (granInt < 0) granInt = 4; // Twelths
                                if (granInt > 4) granInt = 0; // FullStops
                                timer->getStatus()->setGranularity(static_cast<Granularity>(granInt));
                                Logger.log(MYLOG, ELOG_LEVEL_INFO, "adjusted granularity");
                            } else if (currentState == FSTOP_EXPOSURE_CONFIG) {
                                int currentStep = timer->getStatus()->getStep();
                                currentStep += direction;
                                if (currentStep < -3) currentStep = 3;
                                if (currentStep > 3) currentStep = -3;
                                timer->getStatus()->setStep(currentStep);
                                Logger.log(MYLOG, ELOG_LEVEL_INFO, "adjusted step to %d", currentStep);
                            }
                        }
                    }
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}