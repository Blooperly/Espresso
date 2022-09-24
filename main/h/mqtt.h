#pragma once

typedef enum {
	MQTT_STATE_DISCONNECTED,
	MQTT_STATE_CONNECTED,
} MQTT_STATE;

void MQTT_init(void);
void MQTT_appStart(void);
MQTT_STATE MQTT_getState();
bool MQTT_fetchRecentError();
void MQTT_send(uint8_t* buffer, int len);