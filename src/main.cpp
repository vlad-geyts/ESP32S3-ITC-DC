// In a professional MCU application, you rarely want tasks 
// to be completely isolated. You need a way to pass data safely 
// between cores without causing memory corruption (race conditions). 
// In FreeRTOS, the standard tool for this is the Queue.
//
// The Objective
// We will modify your code so that:
//    1. Core 1 (Logic Task) "senses" something (we'll simulate a variable value).
//    2. Core 1 sends that value into a Queue.
//    3. Core 0 (Heartbeat Task) waits for data in that Queue.
//    4. Core 0 changes the LED blink rate based on the data received from Core 1.

#include <Arduino.h>

// Handle for our Queue
// This will hold integers representing the delay in milliseconds
QueueHandle_t blinkDelayQueue;  // define varianble to hold the handle
                                // pointer to a queue structure acting as a handle 
                                // to safely send, receive or manage data in thar specific queue

TaskHandle_t TaskHeartbeat;     // It is defined as a void* (a pointer to void)
                                // It holds the memory address of the Task Control Block (TCB), 
                                // which stores all the information about a task (priority, stack, state, etc.)
TaskHandle_t TaskLogic;

void heartbeatTask(void *pvParameters); // standard function signature for tats in RTOS
                                        // (void *pvParameters) input argument
                                        // "void *"" pointer to void type, whic means the task can receive a 
                                        // pointer to any data type (integer, structure, etc.)
void logicTask(void *pvParameters);

void setup() {
    delay(1000);
    Serial.begin(115200);
    delay(500);

    Serial.println("\n--- Phase 2: Inter-Task Communication ---");

    // 1. Create a Queue: (Queue Length, Size of each item)
    // We want to hold 5 integers max.
    blinkDelayQueue = xQueueCreate(5, sizeof(int));

    if (blinkDelayQueue == NULL) {
        Serial.println("Error creating the queue");
        while(1);
    }

    // Create Heartbeat Task on Core 0 (System Core)
    // xTaskCreatePinnedToCore(heartbeatTask, "Heartbeat", 4096, NULL, 1, &TaskHeartbeat, 0);
    xTaskCreatePinnedToCore(
        heartbeatTask,   // Task function
        "Heartbeat",     // Name
        4096,            // Stack size
        NULL,            // Parameters
        1,               // Priority
        &TaskHeartbeat,  // Task handle
        0                // Core ID
    );

    // Create Logic Task on Core 1 (App Core)
    //xTaskCreatePinnedToCore(logicTask, "Logic", 4096, NULL, 2, &TaskLogic, 1);
    xTaskCreatePinnedToCore(
        logicTask,        // Task function
        "Logic",          // Name
        4096,             // Stack size
        NULL,             // Parameters
        2,                // Higher priority
        &TaskLogic,       // Task handle
        1                 // Core ID
    );
    
}

void loop() {
  // In FreeRTOS, we leave loop() empty or use it for low-priority cleanup.
    vTaskDelete(NULL); 
}

// --- Task Implementations ---
void heartbeatTask(void *pvParameters) {
    const int LED_PIN = 2;
    pinMode(LED_PIN, OUTPUT);
    
    int currentDelay = 1000; // Default 1 second

    for(;;) {
        // 2. Check the Queue for new data (Non-blocking or timed wait)
        // We check if Core 1 sent a new delay value. 
        // xQueueReceive(Queue, buffer, time_to_wait)
        if (xQueueReceive(blinkDelayQueue, &currentDelay, 0) == pdPASS) {
          // xQueueReceive() - function is used to read (and remove) an item from a queue
          // blinkDelayQueue - the handle of the queue to read from
          // &currentDelay - a pointer to the memory where the received item
          // will be copied (the destination variable)
          // 0 (xTicksToWait) - max time to wait for a message if the queue is empty
            Serial.printf("[Core 0] Received NEW blink rate: %d ms\n", currentDelay);
        }

        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
        vTaskDelay(pdMS_TO_TICKS(currentDelay));
    }
}

void logicTask(void *pvParameters) {
    int dummySensorValue = 0;
    int newDelay = 1000;

    for(;;) {
        // Simulate "Logic" deciding to change the blink rate
        // Every 5 iterations, we'll cycle the speed
        dummySensorValue++;
        
        if (dummySensorValue % 5 == 0) {
            newDelay = (newDelay == 1000) ? 100 : 1000; // Toggle between 1s and 100ms
            
            // 3. Send data to the Queue
            // xQueueSend(Queue, data_address, time_to_wait)
            xQueueSend(blinkDelayQueue, &newDelay, portMAX_DELAY);
            Serial.printf("[Core 1] SENT new delay: %d ms to Queue\n", newDelay);
        }

        vTaskDelay(pdMS_TO_TICKS(500)); 
    }
}