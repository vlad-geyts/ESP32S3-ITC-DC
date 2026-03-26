Gemini said:
That is a satisfying milestone, Vlad! Seeing that physical LED toggle confirms that
your FreeRTOS scheduler is healthy and your hardware configuration is correct.

Now, let's step into Phase 2: Inter-Task Communication.
In a professional MCU application, you rarely want tasks to be completely isolated.
You need a way to pass data safely between cores without causing memory corruption 
(race conditions). In FreeRTOS, the standard tool for this is the Queue.

The Objective
We will modify your code so that:

    1. Core 1 (Logic Task) "senses" something (we'll simulate a variable value).
    2. Core 1 sends that value into a Queue.
    3. Core 0 (Heartbeat Task) waits for data in that Queue.
    4. Core 0 changes the LED blink rate based on the data received from Core 1.

Why this is a "Senior EE" approach:
    1. Thread Safety: Queues in FreeRTOS are inherently thread-safe. You don't need 
       to manually lock and unlock Mutexes just to pass a simple value.

    2. Decoupling: heartbeatTask doesn't care how the delay is calculated.
       It just waits for an order from the Queue.

    3. Power Efficiency: While we are using 0 wait time in xQueueReceive (polling),
       in Phase 3 we can make the task block (go to sleep) until a message arrives, saving CPU cycles.

Vlad, once you upload this, you should see the LED blink slowly 5 times, then very rapidly 5 times, cycling forever. Let me know when the Serial monitor confirms the Queue communication is active!