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

	ADMIN_printTasks();
}