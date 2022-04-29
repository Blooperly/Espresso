// Logging
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

// Project Includes
#include "h\bluetooth.h"

// Macros
#define BLUETOOTH_TASK_DELAY (1000 / portTICK_PERIOD_MS)

// File Scope Variables
static const char* TAG = "BLUETOOTH";
static TaskHandle_t task_bluetooth = NULL;
static uint8_t params_bluetooth;

void BLUETOOTH_init() {
    ESP_LOGI(TAG, "BLUETOOTH_init()");

    esp_err_t ret;

    // Initialize NVS Flash (needed for BT library)
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize Bluetooth controller stack configuration structure
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    // Initialize Bluetooth controller stack
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "Bluetooth controller initialization failed");
        return;
    }

    // Enable Bluetooth controller stack (BLE Mode)
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "Bluetooth controller enable failed");
        return;
    }

    // Initialize Bluedroid host stack
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "Bluedroid host initialization failed");
        return;
    }

    // Enable Bluedroid host stack
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "Bluedroid host enable failed");
        return;
    }

    // Register GATTS Event callback
    ret = esp_ble_gatts_register_callback(BLUETOOTH_gatts_event);
    if (ret) {
        ESP_LOGE(TAG, "GATTS callback register failed");
        return;
    }

    // Register GAP Event callback
    ret = esp_ble_gap_register_callback(BLUETOOTH_gap_event);
    if (ret) {
        ESP_LOGE(TAG, "GAP callback register failed");
        return;
    }

    /* IDK what this does yet
    ret = esp_ble_gatts_app_register(gatts_app);
    if (ret){
        ESP_LOGE(TAG, "gatts app register error, error code = %x", ret);
        return;
    }
    
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(512);
    if (local_mtu_ret){
        ESP_LOGE(TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }*/
    
    // Launch bluetooth task
    xTaskCreate(BLUETOOTH_task, "bluetooth", 5000, &params_bluetooth, 10, &task_bluetooth);
}

void BLUETOOTH_task(void* task_params) {

    int task_run_count = 0;

    while (1) {
        task_run_count++;

        if (task_run_count % 10 == 0) ESP_LOGD(TAG, "High Water Mark: %i", uxTaskGetStackHighWaterMark(task_bluetooth));

        vTaskDelay(BLUETOOTH_TASK_DELAY);
    }

    vTaskDelete(task_bluetooth);
}

void BLUETOOTH_gatts_event() {
    ESP_LOGI(TAG, "BLUETOOTH_gatts_event()");
}

void BLUETOOTH_gap_event() {
    ESP_LOGI(TAG, "BLUETOOTH_gap_event()");
}