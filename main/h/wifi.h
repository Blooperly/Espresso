#pragma once

typedef enum {
	WIFI_STATE_DISCONNECTED,
	WIFI_STATE_CONNECTED,
} WIFI_STATE;

void WIFI_init(void);
WIFI_STATE WIFI_getState();