#pragma once

typedef enum {
	HT16K33_COLOR_OFF,
	HT16K33_COLOR_GREEN,
	HT16K33_COLOR_ORANGE,
	HT16K33_COLOR_RED,
} HT16K33_COLOR;

void HT16K33_init();
void HT16K33_setLed(int x, int y, HT16K33_COLOR color);
void HT16K33_leftShift();
void HT16K33_clear();
void HT16K33_setBrightness(uint8_t brightness);