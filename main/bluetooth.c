// Logging
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Project Includes
#include "h\bluetooth.h"
#include "h\main.h"

static const char* TAG = "BLUETOOTH";

extern TaskHandle_t task_bluetooth;

void bluetooth_init() {
    ESP_LOGI(TAG, "bluetooth_init()");
}

void bluetooth_task(void* task_params) {
    static const TickType_t taskDelay = 1000 / portTICK_PERIOD_MS;

    while (1) {
        ESP_LOGD(TAG, "High Water Mark: %i", uxTaskGetStackHighWaterMark(task_bluetooth));

        vTaskDelay(taskDelay);
    }

    vTaskDelete(task_bluetooth);
}