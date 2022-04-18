// Includes
#include <stdio.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

// Project Includes
#include "h/main.h"
#include "h/bluetooth.h"

// Template Includes
#include "esp_spi_flash.h"

// Global Variables
TaskHandle_t task_bluetooth = NULL;
static uint8_t params_bluetooth;

// File Scope Variables
static const char* TAG = "MAIN";

void app_main(void) {
    printAppHeader();

    bluetooth_init();
    
    xTaskCreate(bluetooth_task, "bluetooth", 5000, &params_bluetooth, 10, &task_bluetooth);
    if (task_bluetooth == NULL) printf("TASK NULL\n");

    char task_buffer[500];
    vTaskList(task_buffer);
    printf("%s", task_buffer);
}

static void printAppHeader() {
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

    loggingTest();
}

static void loggingTest() {
    printf("\n");
    ESP_LOGE(TAG, "Logging test: Error");
    ESP_LOGW(TAG, "Logging test: Warning");
    ESP_LOGI(TAG, "Logging test: Info");
    ESP_LOGD(TAG, "Logging test: Debug");
    ESP_LOGV(TAG, "Logging test: Verbose");
    printf("\n");
}