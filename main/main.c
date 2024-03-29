// Main
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include <stdio.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "driver/gpio.h"
#include "led_strip.h"

// Project Includes
#include "h/main.h"
#include "h/admin.h"
#include "h/i2c.h"
#include "h/wifi.h"
#include "h/mqtt.h"
//#include "h/bluetooth.h"
#include "h/ht16k33.h"
#include "h/tmp102.h"
#include "h/task_priority.h"

// Defines
#define APP_TASK_STACK_SIZE 5000
#define HT16K33_TASK_DELAY	(100 / portTICK_RATE_MS)
#define SYSTEM_STATUS_DELAY (1000 / portTICK_RATE_MS)
#define TEMP_SEND_DELAY 	(10000 / portTICK_RATE_MS)

#define SYSVIEW_WRITE_COLUMN	7
#define SYSVIEW_STATUS_ROW		7
#define SYSVIEW_WAVE_TOP		6
#define SYSVIEW_WAVE_BOTTOM		7

// Global Variables

// Local Variables
static const char* TAG = "MAIN";
static TaskHandle_t task_app = NULL;
static uint8_t params_app;

// Local Function Definitions
static void app_task(void* task_params);

// App entry point
void app_main(void) {

	// Global logging
	esp_log_level_set("*", ESP_LOG_INFO);

	// System Initialization
	ADMIN_watchdogInit();
	ADMIN_printAppHeader();

	// Disable RGB LED
	led_strip_t* rgb_led;
	rgb_led = led_strip_init(0, GPIO_NUM_48, 1);
    ESP_ERROR_CHECK(rgb_led->clear(rgb_led, 50));
	led_strip_denit(rgb_led);

	// Init common resources
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	  ESP_ERROR_CHECK(nvs_flash_erase());
	  ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	ESP_ERROR_CHECK(esp_netif_init());

	// Application Initialization
	I2C_init();
	WIFI_init();
	MQTT_init();
	//BLUETOOTH_init();
	HT16K33_init();
	TMP102_init();

	// Launch app task
    xTaskCreate(app_task, "app", APP_TASK_STACK_SIZE, &params_app, TASK_PRIORITY_APP, &task_app);
}

// Module Functions

// Local Functions
static void app_task(void* task_params) {
	TickType_t last_system_status_tick = 0;
	TickType_t last_temp_send_tick = 0;
	
	while (1) {
		TickType_t currentTick = xTaskGetTickCount();
		if (currentTick - last_system_status_tick >= SYSTEM_STATUS_DELAY) {
			last_system_status_tick = currentTick;
			
			// Prepare Sysview
			HT16K33_leftShift(SYSVIEW_STATUS_ROW);

			// Write new Sysview pixels
			if (WIFI_fetchRecentError() || MQTT_fetchRecentError()) {
				HT16K33_setLed(SYSVIEW_WRITE_COLUMN, SYSVIEW_STATUS_ROW, HT16K33_COLOR_RED);
			} else {
				HT16K33_setLed(SYSVIEW_WRITE_COLUMN, SYSVIEW_STATUS_ROW, HT16K33_COLOR_GREEN);
			}

			// Display Temperature
			HT16K33_drawTemp(TMP102_readTemp());
		}

		if (currentTick - last_temp_send_tick >= TEMP_SEND_DELAY) {
			last_temp_send_tick = currentTick;
			uint8_t temp = TMP102_readTemp();
			ESP_LOGI(TAG, "Sending temp = %02X", temp);
			MQTT_send(&temp, 1);
		}

		vTaskDelay(HT16K33_TASK_DELAY);
	}
}