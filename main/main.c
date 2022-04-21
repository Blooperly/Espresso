// Logging
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

// Project Includes
#include "h/main.h"
#include "h/admin.h"
#include "h/bluetooth.h"

// Global Variables

// File Scope Variables
static const char* TAG = "MAIN";

void app_main(void) {
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    ADMIN_printAppHeader();

#if defined(CONFIG_BT_ENABLED) && defined(CONFIG_BLUEDROID_ENABLED)
    // Bluetooth Initialization
    bluetooth_init();
#else
    ESP_LOGE(TAG, "Bluetooth feature disabled by sdkconfig");
#endif

    ADMIN_printTasks();
}