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
TaskHandle_t beeperTask;

// Queues for inter-task communication
QueueHandle_t inputQueue;
QueueHandle_t displayQueue;
QueueHandle_t beeperQueue;

SemaphoreHandle_t tm1638Mutex;

// Shared variables
float baseTime = BASE_TIME_DEFAULT;
int baseFStop = BASE_FSTOP_DEFAULT;
int currentFStop = BASE_FSTOP_DEFAULT;
float calculatedTime = BASE_TIME_DEFAULT;
long valueBuffer = 0;

// Function prototypes
void inputHandler(void *pvParameters);
void displayUpdate(void *pvParameters);
void exposureTimer(void *pvParameters);
//void countdownExposureTime(double exposureTime);
void stateManager(void *pvParameters);
void beeper(void *pvParameters);


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

    Logger.log(MYLOG, ELOG_LEVEL_INFO, "creating queues and tasks...");

    // Create queues
    inputQueue = xQueueCreate(10, sizeof(QueueItem));
    displayQueue = xQueueCreate(10, sizeof(QueueItem));
    beeperQueue = xQueueCreate(10, sizeof(QueueItem));

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
                              new Beeper(BEEPER_PIN),
                              inputQueue,
                              displayQueue,
                              beeperQueue);


    Logger.log(MYLOG, ELOG_LEVEL_INFO, "initializing rotary encoders...");                              
    timer->setup();    

    // Create tasks
    xTaskCreate(inputHandler, "InputHandler", 2048, NULL, 2, &inputHandlerTask);
    xTaskCreate(displayUpdate, "DisplayUpdate", 2048, NULL, 3, &displayUpdateTask);
    xTaskCreate(exposureTimer, "ExposureTimer", 2048, NULL, 1, &exposureTimerTask);
    xTaskCreate(stateManager, "StateManager", 2048, NULL, 1, &stateManagerTask);
    xTaskCreate(beeper, "Beeper", 2048, NULL, 3, &beeperTask);
    Logger.log(MYLOG, ELOG_LEVEL_INFO, "finished initialization...");

     //testExposureCalculator();
    timer->getDisplay()->setLEDState(2^7);
    timer->getDisplay()->setLEDs(); 
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
        timer->handleInput();
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void displayUpdate(void *pvParameters) {
    QueueItem *msg = new QueueItem;

    // The displayUpdate task basically only takes the latest display buffer and transmits it
    // to the TM1638 display. The display buffer is updated by the state manager task whenever there 
    // is a change in the system state that requires a display update (for example mode change, 
    // encoder value change, timer tick etc.). The displayUpdate task runs in an infinite loop and updates 
    // the display at a regular interval (for example every 100ms) to ensure that any changes to the 
    // display buffer are reflected on the TM1638 in a timely manner. This separation of concerns 
    // allows the state manager to focus on managing the system state and preparing the display 
    // buffer, while the displayUpdate task handles the actual communication with the TM1638 display.
    while (true) 
    {
        timer->getDisplay()->setDisplay(timer->getDisplayBuffer());
        timer->getDisplay()->setLEDs();

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void exposureTimer(void *pvParameters) 
{

    while (true) 
    {
        timer->exposureControl();

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

void beeper(void *pvParameters)
{
    QueueItem *msg = new QueueItem;

    while (true) 
    {
        if (xQueueReceive(beeperQueue, msg, portMAX_DELAY)) 
        {

            timer->handleBeeper(msg);            
        }
    }
}
