// TMP102
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
#include "h/tmp102.h"
#include "h/i2c.h"
#include "h/task_priority.h"

// Defines

// Local Variables
TMP102_TEMP_MODE temp_mode;

// Local Function Definitions

// Module Functions
void TMP102_init() {
	temp_mode = TMP102_TEMP_MODE_F;
	return;
}

uint8_t TMP102_readTemp() {
	uint8_t tmp102_data[2];
	I2C_read(I2C_DEVICE_tmp102, tmp102_data, 2);

	bool temp_neg = tmp102_data[0] & 0x80;

	if (temp_mode == TMP102_TEMP_MODE_C) {
		if (!temp_neg) {
			return (((((int)tmp102_data[0] << 4) | ((int)tmp102_data[1] >> 4)) & 0x0fff) * 625) / 10000;
		} else {
			// *J* handle negative temps
			return 0;
		}
	} else if (temp_mode == TMP102_TEMP_MODE_F) {
		if (!temp_neg) {
			return (((((((int)tmp102_data[0] << 4) | ((int)tmp102_data[1] >> 4)) & 0x0fff) * 625 * 9) / (10000 * 5)) + 32);
		} else {
			// *J* handle negative temps
			return 0;
		}
	}

	return 0;
}

// Local Functions