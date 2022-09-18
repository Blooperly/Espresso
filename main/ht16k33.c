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
static void ht16k33_task(void* task_params);
static void display(uint8_t* buffer);

// Module Functions
void HT16K33_init() {
	// Initial Display
	uint8_t cmd_ht16k33_start[1] 			= {0x21};
	uint8_t cmd_ht16k33_display_on[1] 		= {0x81};

	I2C_write(I2C_DEVICE_ht16k33, cmd_ht16k33_start, 1);
	I2C_write(I2C_DEVICE_ht16k33, cmd_ht16k33_display_on, 1);
	
    xTaskCreate(ht16k33_task, "ht16k33", 5000, &params_ht16k33, 5, &task_ht16k33);
}

void HT16K33_setLed(int x, int y, HT16K33_COLOR color) {
	int target_row = (y * 2);

	if (color == HT16K33_COLOR_GREEN) {
		display_data[target_row] 		|= 1 << x;
		display_data[target_row + 1] 	&= ~(1 << x);
	} else if (color == HT16K33_COLOR_ORANGE) {
		display_data[target_row] 		|= 1 << x;
		display_data[target_row + 1] 	|= 1 << x;
	} else if (color == HT16K33_COLOR_RED) {
		display_data[target_row] 		&= ~(1 << x);
		display_data[target_row + 1] 	|= 1 << x;
	} else if (color == HT16K33_COLOR_OFF) {
		display_data[target_row] 		&= ~(1 << x);
		display_data[target_row + 1] 	&= ~(1 << x);
	}
}

// Local Functions
static void ht16k33_task(void* task_params) {
	int blank_tier = 0;
	int color_adjusted = 0;
	while (1) {
		for (int i = 0; i < 8; i++) {
			for (int j = 0; j < 8; j++) {
				// Tier 3
				if (i == 0 || i == 7 || j == 0 || j == 7) {
					color_adjusted = blank_tier;
					if (color_adjusted > 3) color_adjusted -= 4;
					HT16K33_setLed(i, j, color_adjusted);
				}

				// Tier 2
				else if (i == 1 || i == 6 || j == 1 || j == 6) {
					color_adjusted = blank_tier + 1;
					if (color_adjusted > 3) color_adjusted -= 4;
					HT16K33_setLed(i, j, color_adjusted);
				}

				// Tier 1
				else if (i == 2 || i == 5 || j == 2 || j == 5) {
					color_adjusted = blank_tier + 2;
					if (color_adjusted > 3) color_adjusted -= 4;
					HT16K33_setLed(i, j, color_adjusted);
				}

				// Tier 0
				else {
					color_adjusted = blank_tier + 3;
					if (color_adjusted > 3) color_adjusted -= 4;
					HT16K33_setLed(i, j, color_adjusted);
				}
			}
		}

		blank_tier++;
		if (blank_tier == 4) blank_tier = 0;
		
		display(display_data);
		vTaskDelay(HT16K33_TASK_DELAY);
	}
}

static void display(uint8_t* buffer) {
	uint8_t cmd[HT16K33_DISPLAY_BUFFER_LEN + 1] = {0x00};
	memcpy(&cmd[1], buffer, HT16K33_DISPLAY_BUFFER_LEN);
	I2C_write(I2C_DEVICE_ht16k33, cmd, HT16K33_DISPLAY_BUFFER_LEN + 1);
}