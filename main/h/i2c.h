#pragma once

typedef enum {
	I2C_DEVICE_ht16k33 = 0x70,
} I2C_DEVICE;

void I2C_init();
esp_err_t I2C_write(I2C_DEVICE device, uint8_t* data, int len);