#include "task_power_optimize.h"

PowerState current_power_state = POWER_NORMAL;
unsigned long buttonPressStartTime = 0;
static bool bootHandled = false;

void task_power_optimize(void *pvParameters)
{
    pinMode(BOOT_PIN, INPUT_PULLUP);

    Serial.println("Power Optimize Task Started");
    Serial.println("Modes: NORMAL (100% load) ↔ POWER_OPTIMIZE (50% reduced)");
    Serial.println("Hold BOOT button > 1s to switch modes");

    while (1)
    {
        switch (current_power_state)
        {

        case POWER_NORMAL:
            // NORMAL → POWER_OPTIMIZE: Hold BOOT > 1s
            if (digitalRead(BOOT_PIN) == LOW)
            {
                if (buttonPressStartTime == 0)
                {
                    buttonPressStartTime = millis();
                }
                else if ((millis() - buttonPressStartTime > 1000) && !bootHandled)
                {
                    // Give semaphore to notify other tasks
                    xSemaphoreGive(xBinarySemaphorePowerOptimize);
                    current_power_state = POWER_OPTIMIZE;
                    buttonPressStartTime = 0;
                    bootHandled = true;
                    Serial.println("\n⚡ POWER_OPTIMIZE MODE: Reducing load (50%), dimming displays...");
                }
            }
            else
            {
                buttonPressStartTime = 0;
                bootHandled = false;
            }
            break;

        case POWER_OPTIMIZE:
            // POWER_OPTIMIZE → NORMAL: Hold BOOT > 1s
            if (digitalRead(BOOT_PIN) == LOW)
            {
                if (buttonPressStartTime == 0)
                {
                    buttonPressStartTime = millis();
                }
                else if ((millis() - buttonPressStartTime > 1000) && !bootHandled)
                {
                    xSemaphoreGive(xBinarySemaphoreNormalMode);
                    current_power_state = POWER_NORMAL;
                    buttonPressStartTime = 0;
                    bootHandled = true;
                    Serial.println("\n🔋 NORMAL MODE: Full load (100%), all tasks resumed!");
                }
            }
            else
            {
                buttonPressStartTime = 0;
                bootHandled = false;
            }
            break;

        default:
            current_power_state = POWER_NORMAL;
            break;
        }

        // Print current state every 10 seconds
        static unsigned long lastPrint = 0;
        if (millis() - lastPrint > 10000)
        {
            lastPrint = millis();
            const char *stateStr[] = {"100% NORMAL", "50% OPTIMIZE"};
            Serial.printf("[Power State] %s | Anomaly: %s | WiFi: %s\n",
                          stateStr[current_power_state],
                          anomaly_detected ? "YES" : "NO",
                          isWifiConnected ? "CONNECTED" : "OFFLINE");
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
