// I2C
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "driver/i2c.h"
#include "hal/i2c_types.h"
#include "hal/gpio_types.h"

// Project Includes
#include "h/i2c.h"
#include "h/gpio.h"

// Defines
#define I2C_MASTER_FREQ_HZ 			400000 	// Limited by ht16k33
#define I2C_BUFFER_NA				0		// Buffer N/A for master port
#define I2C_MASTER_TIMEOUT_MS		1		// Timeout

// Local Variables
static const char *TAG = "I2C";

// Local Function Definitions

// Module Functions
void I2C_init() {
	// Initialize I2C driver
	static i2c_config_t i2c_master_config = {
		.mode 				= I2C_MODE_MASTER,
		.sda_io_num 		= GPIO_I2C_DATA,
		.scl_io_num 		= GPIO_I2C_CLK,
		.sda_pullup_en 		= GPIO_PULLUP_DISABLE,
		.scl_pullup_en 		= GPIO_PULLUP_DISABLE,
		.master.clk_speed 	= I2C_MASTER_FREQ_HZ,
	};

	ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_master_config));
	ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, i2c_master_config.mode, I2C_BUFFER_NA, I2C_BUFFER_NA, 0));
}

esp_err_t I2C_write(I2C_DEVICE device, uint8_t* data, int len) {
	//printf("\n\nI2C sent to 0x%02X:", device);
	//for (int i = 0; i < len; i++) {
	//	printf(" %02X", data[i]);
	//}
	//printf("\n\n");
	
	return i2c_master_write_to_device(I2C_NUM_0, device, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_RATE_MS);
}