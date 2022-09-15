// Command
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include <stdio.h>
#include "esp_log.h"

// Project Includes
#include "h\command.h"

// Defines

// Local Variables
static const char *TAG = "CMD";

void COMMAND_parseCommand(char* cmdData, uint16_t cmdLen) {
	ESP_LOGI(TAG, "Command - ID: %02X%02X", cmdData[0], cmdData[1]);
}