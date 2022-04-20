// Logging
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Project Includes
#include "h\admin.h"
#include "h\main.h"

static const char* TAG = "ADMIN";

void ADMIN_printTasks() {
    //char task_buffer[700];
    //vTaskList(task_buffer);
    //printf("%s", task_buffer);
}