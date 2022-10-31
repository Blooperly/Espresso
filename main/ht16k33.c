// HT16K33
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Project Includes
#include "h/ht16k33.h"
#include "h/i2c.h"
#include "h/task_priority.h"

// Defines
#define HT16K33_TASK_DELAY			(100 / portTICK_RATE_MS)
#define HT16K33_TASK_STACK_SIZE		4000
#define HT16K33_DISPLAY_BUFFER_LEN 	16
#define HT16K33_INITIAL_BRIGHTNESS	0x04
#define HT16K33_HEIGHT_WIDTH		8

#define FONT_CHARACTERS				10
#define FONT_BYTES_PER_CHARACTER 	16
#define FONT_WIDTH					3
#define FONT_HEIGHT					5

#define TEMP_HIGH_COORDINATE_X		0
#define TEMP_HIGH_COORDINATE_Y		0
#define TEMP_LOW_COORDINATE_X		4
#define TEMP_LOW_COORDINATE_Y		0

// Local Variables
static const char *TAG = "HT16K33";
static TaskHandle_t task_ht16k33 = NULL;
static uint8_t params_ht16k33;

static uint8_t display_data[HT16K33_DISPLAY_BUFFER_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	// 3x5 Font
static uint8_t font_bitmap[FONT_CHARACTERS * FONT_BYTES_PER_CHARACTER] = {
	0x06, 0x06, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0
	0x02, 0x02, 0x03, 0x03, 0x02, 0x02, 0x02, 0x02, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 1
	0x07, 0x07, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 2
	0x07, 0x07, 0x04, 0x04, 0x06, 0x06, 0x04, 0x04, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 3
	0x05, 0x05, 0x05, 0x05, 0x07, 0x07, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 4
	0x07, 0x07, 0x01, 0x01, 0x07, 0x07, 0x04, 0x04, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 5
	0x06, 0x06, 0x01, 0x01, 0x07, 0x07, 0x05, 0x05, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 6
	0x07, 0x07, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 7
	0x06, 0x06, 0x05, 0x05, 0x07, 0x07, 0x05, 0x05, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 8
	0x06, 0x06, 0x05, 0x05, 0x07, 0x07, 0x04, 0x04, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 9
	};

// Local Function Definitions
static void ht16k33_task(void* task_params);
static void display(uint8_t* buffer);
static void writeMaskedRect(uint8_t* data, uint8_t x, uint8_t y, uint8_t w, uint8_t h);
static uint8_t rowBitmask(uint8_t x, uint8_t width);

// Module Functions
void HT16K33_init() {
	// Initial Display
	uint8_t cmd_ht16k33_start[1] 			= {0x21};
	uint8_t cmd_ht16k33_display_on[1] 		= {0x81};

	I2C_write(I2C_DEVICE_ht16k33, cmd_ht16k33_start, 1);
	I2C_write(I2C_DEVICE_ht16k33, cmd_ht16k33_display_on, 1);
	
	HT16K33_clear();
	HT16K33_setBrightness(HT16K33_INITIAL_BRIGHTNESS);

	// Start display refresh task
    xTaskCreate(ht16k33_task, "ht16k33", HT16K33_TASK_STACK_SIZE, &params_ht16k33, TASK_PRIORITY_HT16K33, &task_ht16k33);
}

void HT16K33_setLed(int x, int y, HT16K33_COLOR color) {
	uint8_t target_row = (y * 2);
	uint8_t target_column = 1 << x;

	if (color == HT16K33_COLOR_GREEN) {
		display_data[target_row] 		|= target_column;
		display_data[target_row + 1] 	&= ~target_column;
	} else if (color == HT16K33_COLOR_ORANGE) {
		display_data[target_row] 		|= target_column;
		display_data[target_row + 1] 	|= target_column;
	} else if (color == HT16K33_COLOR_RED) {
		display_data[target_row] 		&= ~target_column;
		display_data[target_row + 1] 	|= target_column;
	} else if (color == HT16K33_COLOR_OFF) {
		display_data[target_row] 		&= ~target_column;
		display_data[target_row + 1] 	&= ~target_column;
	}
}

void HT16K33_leftShift(int row) {
	uint8_t target_row = (row * 2);
	display_data[target_row] = (display_data[target_row] >> 1) & 0xFF;
	display_data[target_row + 1] = (display_data[target_row + 1] >> 1) & 0xFF;
}

void HT16K33_clear() {
	for (int i = 0; i < HT16K33_DISPLAY_BUFFER_LEN; i++) {
		display_data[i] = 0x00;
	}
}

void HT16K33_setBrightness(uint8_t brightness) {
	uint8_t cmd_ht16k33_brightness = 0xE0 | (brightness & 0x0F);
	I2C_write(I2C_DEVICE_ht16k33, &cmd_ht16k33_brightness, 1);
}

void HT16K33_drawTemp(int temp) {
	if (temp < 0 || temp > 99) {
		// Temperature out of range
		return;
	}

	int temp_high = temp / 10;
	int temp_low = temp % 10;

	writeMaskedRect(&font_bitmap[temp_high * FONT_BYTES_PER_CHARACTER], TEMP_HIGH_COORDINATE_X, TEMP_HIGH_COORDINATE_Y, FONT_WIDTH, FONT_HEIGHT);
	writeMaskedRect(&font_bitmap[temp_low * FONT_BYTES_PER_CHARACTER], TEMP_LOW_COORDINATE_X, TEMP_LOW_COORDINATE_Y, FONT_WIDTH, FONT_HEIGHT);
}

// Local Functions
static void ht16k33_task(void* task_params) {

	while (1) {
#ifdef DISPLAY_TEST
	// Display test
		static int blank_tier = 0;
		static int color_adjusted = 0;
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
#endif
		
		display(display_data);
		vTaskDelay(HT16K33_TASK_DELAY);
	}
}

static void display(uint8_t* buffer) {
	uint8_t cmd[HT16K33_DISPLAY_BUFFER_LEN + 1] = {0x00};
	memcpy(&cmd[1], buffer, HT16K33_DISPLAY_BUFFER_LEN);
	I2C_write(I2C_DEVICE_ht16k33, cmd, HT16K33_DISPLAY_BUFFER_LEN + 1);
}

static void writeMaskedRect(uint8_t* data, uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
	// Writes a masked masked display buffer to display data
	if (w == 0 || h == 0 || x > 7 || y > 7) {
		return;
	}

	uint8_t bitmask = rowBitmask(x, w);

	for (int i = 0; (i < h) && (i + y < HT16K33_HEIGHT_WIDTH); i++) {
		display_data[((i + y) * 2)    ] = ((data[(i * 2)    ] << x) & bitmask) | (display_data[((i + y) * 2)    ] & ~bitmask);
		display_data[((i + y) * 2) + 1] = ((data[(i * 2) + 1] << x) & bitmask) | (display_data[((i + y) * 2) + 1] & ~bitmask);
	}
}

static uint8_t rowBitmask(uint8_t x, uint8_t width) {
	uint8_t out = 0;

	for (uint8_t i = 0; (i < width) && (i < HT16K33_HEIGHT_WIDTH); i++) {
		out |= 0x01 << (i + x);
	}

	return out;
}