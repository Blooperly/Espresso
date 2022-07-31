#pragma once

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

void BLUETOOTH_init();
void BLUETOOTH_task(void* task_params);
void BLUETOOTH_gap_event(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
void BLUETOOTH_gatts_event(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

enum GATTS_APP_LIST {
    GATTS_APP,
    GATTS_APP_LAST
};