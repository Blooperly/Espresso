// Main
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include <stdio.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"

// Project Includes
#include "h/main.h"
#include "h/admin.h"
#include "h/wifi.h"
#include "h/mqtt.h"
//#include "h/bluetooth.h"

// Global Variables

// File Scope Variables
static const char* TAG = "MAIN";

// App entry point
void app_main(void) {

    // Global logging
    #ifdef DEBUG
    esp_log_level_set("*", ESP_LOG_VERBOSE);
    #else
    esp_log_level_set("*", ESP_LOG_INFO);
    #endif

    // App Initialization
    ADMIN_watchdogInit();
    ADMIN_printAppHeader();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    WIFI_init();
    MQTT_init();
    //BLUETOOTH_init();

    ADMIN_printTasks();
}