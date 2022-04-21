// Logging
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Project Includes
#include "h\admin.h"
#include "h\main.h"

static const char* TAG = "ADMIN";

void ADMIN_printAppHeader() {
    printf("\n\n\n\
        #####################################################################################\n\
        #                                    Espresso                                       #\n\
        #                                     v0.0.1                                        #\n\
        #####################################################################################\n\n\n");

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    printf(" Model: %s\n", CONFIG_IDF_TARGET);
    printf(" Features: WiFi%s%s\n", (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "", (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    printf(" Cores: %i\n", chip_info.cores);
    printf(" Revision: %i\n", chip_info.revision);

    ADMIN_loggingTest();
}

void ADMIN_loggingTest() {
    printf("\n");
    ESP_LOGE(TAG, "Logging test: Error");
    ESP_LOGW(TAG, "Logging test: Warning");
    ESP_LOGI(TAG, "Logging test: Info");
    ESP_LOGD(TAG, "Logging test: Debug");
    ESP_LOGV(TAG, "Logging test: Verbose");
    printf("\n");
}

void ADMIN_printTasks() {
    char task_buffer[700];
    vTaskList(task_buffer);
    printf("%s", task_buffer);
}