#pragma once

#define NETWORK_MAC_SIZE		6

typedef enum {
	WIFI_STATE_DISCONNECTED,
	WIFI_STATE_CONNECTED,
} WIFI_STATE;

void WIFI_init(void);
uint8_t* WIFI_getNetworkMac();
WIFI_STATE WIFI_getState();
bool WIFI_fetchRecentError();