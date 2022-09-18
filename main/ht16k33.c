// HT16K33
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Project Includes
#include "h/ht16k33.h"
#include "h/i2c.h"

// Defines
#define HT16K33_TASK_DELAY			(100 / portTICK_RATE_MS)
#define HT16K33_DISPLAY_BUFFER_LEN 	16

// Local Variables
static const char *TAG = "HT16K33";
static TaskHandle_t task_ht16k33 = NULL;
static uint8_t params_ht16k33;

static uint8_t display_data[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Local Function Definitions
void ht16k33_task(void* task_params);
void display(uint8_t* buffer);

// Module Functions
void HT16K33_init() {
	// Initial Display
	uint8_t cmd_ht16k33_start[1] 			= {0x21};
	uint8_t cmd_ht16k33_display_on[1] 		= {0x81};

	I2C_write(I2C_DEVICE_ht16k33, cmd_ht16k33_start, 1);
	I2C_write(I2C_DEVICE_ht16k33, cmd_ht16k33_display_on, 1);
	
    xTaskCreate(ht16k33_task, "ht16k33", 5000, &params_ht16k33, 5, &task_ht16k33);
}

// Local Functions
void ht16k33_task(void* task_params) {
	while (1) {
		for (int i = 0; i < 16; i++) {
			for (int j = 0; j < 8; j++) {
				display_data[i] = display_data[i] ^ (1 << j);
				display(display_data);
				vTaskDelay(HT16K33_TASK_DELAY);
			}
		}
	}
}

void display(uint8_t* buffer) {
	uint8_t cmd[HT16K33_DISPLAY_BUFFER_LEN + 1] = {0x00};
	memcpy(&cmd[1], buffer, HT16K33_DISPLAY_BUFFER_LEN);
	I2C_write(I2C_DEVICE_ht16k33, cmd, HT16K33_DISPLAY_BUFFER_LEN + 1);
}