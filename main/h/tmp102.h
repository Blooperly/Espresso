#pragma once

typedef enum {
	TMP102_TEMP_MODE_C,
	TMP102_TEMP_MODE_F,
} TMP102_TEMP_MODE;

void TMP102_init();
uint8_t TMP102_readTemp();