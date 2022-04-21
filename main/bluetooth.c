// Logging
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Project Includes
#include "h\bluetooth.h"
#include "h\main.h"

// Macros
#define BLUETOOTH_TASK_DELAY (1000 / portTICK_PERIOD_MS)

// File Scope Variables
static const char* TAG = "BLUETOOTH";
static TaskHandle_t task_bluetooth = NULL;
static uint8_t params_bluetooth;

void bluetooth_init() {
    ESP_LOGI(TAG, "bluetooth_init()");
    
    xTaskCreate(bluetooth_task, "bluetooth", 5000, &params_bluetooth, 10, &task_bluetooth);
}

void bluetooth_task(void* task_params) {

    int task_run_count = 0;

    while (1) {
        task_run_count++;

        if (task_run_count % 10 == 0) ESP_LOGD(TAG, "High Water Mark: %i", uxTaskGetStackHighWaterMark(task_bluetooth));

        vTaskDelay(BLUETOOTH_TASK_DELAY);
    }

    vTaskDelete(task_bluetooth);
}