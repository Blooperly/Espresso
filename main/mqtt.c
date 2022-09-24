// MQTT
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

// Project Includes
#include "h/mqtt.h"
#include "h/wifi.h"
#include "h/command.h"
#include "h/settings.h"
#include "h/task_priority.h"

// Defines
#define DEFAULT_MQTT_TOPIC  "Espresso"
#define MAC_STRING_LEN      18

// Local Variables
static const char *TAG = "MQTT";

typedef struct {
	MQTT_STATE 	                eState;
	bool		                bRecentError;
    esp_mqtt_client_handle_t    client;
    char                        topic[sizeof(DEFAULT_MQTT_TOPIC) + MAC_STRING_LEN];
} MQTT_CONTEXT;

static MQTT_CONTEXT sMqttContext;
static MQTT_CONTEXT* pMqttContext;

// todo: Configurable MQTT broker address
static esp_mqtt_client_config_t mqtt_cfg = {
    .uri = DEFAULT_MQTT_ADDR,
};

// Local Function Definitions
static void mqttEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
static void log_error_if_nonzero(const char *message, int error_code);
static void setState(MQTT_STATE eState);

// Module Functions
void MQTT_init(void) {
	// Initialize Mqtt Context Pointer
	pMqttContext = &sMqttContext;
}

void MQTT_appStart(void) {
    pMqttContext->client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(pMqttContext->client, ESP_EVENT_ANY_ID, mqttEventHandler, NULL);
    esp_mqtt_client_start(pMqttContext->client);
}

void MQTT_send(uint8_t* buffer, int len) {
    if (MQTT_getState() == MQTT_STATE_CONNECTED) {
        esp_mqtt_client_publish(pMqttContext->client, pMqttContext->topic, (char*)buffer, len, 0, 0);
    }
}

MQTT_STATE MQTT_getState() {
	return pMqttContext->eState;
}

bool MQTT_fetchRecentError() {
	if (pMqttContext->bRecentError) {
		pMqttContext->bRecentError = false;
		return true;
	}

	if (MQTT_getState() != MQTT_STATE_CONNECTED) {
		return true;
	}

	return false;
}

// Local Functions
static void mqttEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "mqttEventHandler: MQTT_EVENT_CONNECTED");

            // Subscribe to Espresso/MAC
            setState(MQTT_STATE_CONNECTED);
            uint8_t* pawWifiMac = WIFI_getNetworkMac();
            sprintf(pMqttContext->topic, "%s/%02X:%02X:%02X:%02X:%02X:%02X", DEFAULT_MQTT_TOPIC, pawWifiMac[0], pawWifiMac[1], pawWifiMac[2], pawWifiMac[3], pawWifiMac[4], pawWifiMac[5]);
            msg_id = esp_mqtt_client_subscribe(client, pMqttContext->topic, 0);
            ESP_LOGI(TAG, "SUBSCRIBED: %s", pMqttContext->topic);
            break;

        case MQTT_EVENT_DISCONNECTED:
			ESP_LOGI(TAG, "mqttEventHandler: MQTT_EVENT_DISCONNECTED");
            setState(MQTT_STATE_DISCONNECTED);
            break;

        case MQTT_EVENT_SUBSCRIBED:
			ESP_LOGI(TAG, "mqttEventHandler: MQTT_EVENT_SUBSCRIBED, msg_id = %d", event->msg_id);
            break;

        case MQTT_EVENT_UNSUBSCRIBED:
			ESP_LOGI(TAG, "mqttEventHandler: MQTT_EVENT_UNSUBSCRIBED, msg_id = %d", event->msg_id);
            break;

        case MQTT_EVENT_PUBLISHED:
			ESP_LOGI(TAG, "mqttEventHandler: MQTT_EVENT_PUBLISHED, msg_id = %d", event->msg_id);
            break;

        case MQTT_EVENT_DATA:
			ESP_LOGI(TAG, "mqttEventHandler: MQTT_EVENT_DATA");

            // Log data
            char buffer[MQTT_MAX_DATA_LEN];
            printf("\n\t%.*s: ", event->topic_len, event->topic);
            memcpy(buffer, event->data, event->data_len);
            for (int i = 0; i < event->data_len; i++) {
                printf("%02X", buffer[i]);
            }
            printf("\n\n");

            // Process data
            COMMAND_parseCommand(buffer, event->data_len);

            break;

        case MQTT_EVENT_ERROR:
			ESP_LOGE(TAG, "mqttEventHandler: MQTT_EVENT_ERROR");
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGE(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }
            break;

        default:
            ESP_LOGW(TAG, "mqttEventHandler: UNKNOWN %d", event->event_id);
    }
}

static void setState(MQTT_STATE eState) {
	pMqttContext->eState = eState;
}

static void log_error_if_nonzero(const char *message, int error_code) {
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}
