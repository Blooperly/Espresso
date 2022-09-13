// Main
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include <stdio.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"

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

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());

    WIFI_init();
    MQTT_init();
    //BLUETOOTH_init();

    ADMIN_printTasks();
}