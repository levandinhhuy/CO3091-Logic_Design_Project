#ifndef __TASK_POWER_OPTIMIZE_H__
#define __TASK_POWER_OPTIMIZE_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_sleep.h"
#include "global.h"

enum PowerState {
    POWER_NORMAL,        // Tất cả task chạy bình thường (100% tải)
    POWER_OPTIMIZE       // Giảm task, tắt một số hiển thị (50% tải)
};

#define BOOT_PIN 0        // GPIO0 - BOOT button

extern PowerState current_power_state;
extern SemaphoreHandle_t xBinarySemaphorePowerOptimize;
extern SemaphoreHandle_t xBinarySemaphoreNormalMode;

void task_power_optimize(void *pvParameters);

#endif
