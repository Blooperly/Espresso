// Logging
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include <stdio.h>
#include "esp_log.h"

// Project Includes
#include "h/main.h"
#include "h/admin.h"
#include "h/bluetooth.h"

// Global Variables

// File Scope Variables
static const char* TAG = "MAIN";

// App entry point
void app_main(void) {

    #ifdef DEBUG
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    #else
    esp_log_level_set("*", ESP_LOG_INFO);
    #endif

    ADMIN_watchdogInit();
    ADMIN_printAppHeader();

    // Bluetooth Initialization
    #if defined(CONFIG_BT_ENABLED) && defined(CONFIG_BLUEDROID_ENABLED)
    bluetooth_init();
    #else
    ESP_LOGE(TAG, "Bluetooth feature disabled by sdkconfig");
    #endif

    ADMIN_printTasks();
}