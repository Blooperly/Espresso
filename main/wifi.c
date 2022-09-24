// Wifi
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

// Project Includes
#include "h/wifi.h"
#include "h/mqtt.h"
#include "h/settings.h"

// Defines
#define WIFI_MAXIMUM_RETRY  	5

// Local Variables
static const char *TAG = "WIFI";

typedef struct {
	WIFI_STATE 	eState;
	uint8_t		uRetryCount;
	uint8_t 	awWifiMac[NETWORK_MAC_SIZE];
	bool		bRecentError;
} WIFI_CONTEXT;

static WIFI_CONTEXT sWifiContext;
static WIFI_CONTEXT* pWifiContext;

static esp_event_handler_instance_t handler_instance_wifi = NULL;
static esp_event_handler_instance_t handler_instance_ip = NULL;
static esp_netif_t* wifi_netif = NULL;

// Wifi Connection Parameters
// todo: Handle WiFi configuration over bluetooth?
// todo: Load saved WiFi configuration from NVS
static wifi_config_t wifi_config = {
	.sta = {
		.ssid = DEFAULT_WIFI_SSID,
		.password = DEFAULT_WIFI_PASS,
		.scan_method = WIFI_FAST_SCAN,
		.threshold.authmode = WIFI_AUTH_WPA2_PSK, 	// Arch: Remove this to enable connection to less secure AP's

		.pmf_cfg = {
			.capable = true,
			.required = false
		},
	},
};

// Local Function Definitions
static void wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void setState(WIFI_STATE eState);
static void disconnectHandler();

// Module Functions
void WIFI_init(void) {
	// Initialize Wifi Context Pointer
	pWifiContext = &sWifiContext;
	
	// Initialize Wifi
    wifi_netif = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	pWifiContext->uRetryCount = 0;

	// Register Events
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, NULL, &handler_instance_wifi));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, NULL, &handler_instance_ip));

	// Run Wifi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

	ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, pWifiContext->awWifiMac));
	ESP_LOGI(TAG, "Starting WiFi for %02X:%02X:%02X:%02X:%02X:%02X", pWifiContext->awWifiMac[0], pWifiContext->awWifiMac[1], pWifiContext->awWifiMac[2], pWifiContext->awWifiMac[3], pWifiContext->awWifiMac[4], pWifiContext->awWifiMac[5]);

    ESP_ERROR_CHECK(esp_wifi_start());
}

uint8_t* WIFI_getNetworkMac() {
	return &pWifiContext->awWifiMac;
}

WIFI_STATE WIFI_getState() {
	return pWifiContext->eState;
}

bool WIFI_fetchRecentError() {
	if (pWifiContext->bRecentError) {
		pWifiContext->bRecentError = false;
		return true;
	}

	if (WIFI_getState() != WIFI_STATE_CONNECTED) {
		return true;
	}

	return false;
}

// Local Functions
static void wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
	// WiFi Events
	if (event_base == WIFI_EVENT) {
		switch (event_id) {
			case WIFI_EVENT_SCAN_DONE:
				ESP_LOGI(TAG, "wifiEventHandler: WIFI_EVENT_SCAN_DONE");
				// todo: Implement scan for AP's. Trusted AP list?
				break;

			case WIFI_EVENT_STA_START:
				ESP_LOGI(TAG, "wifiEventHandler: WIFI_EVENT_STA_START");
        		esp_wifi_connect(); // Initiate connection with communicating AP
				break;

			case WIFI_EVENT_STA_STOP:
				ESP_LOGI(TAG, "wifiEventHandler: WIFI_EVENT_STA_STOP");
				// Arch: We could disable WiFi station mode, but I don't think there is a reason for an IoT device
				break;

			case WIFI_EVENT_STA_CONNECTED:
				ESP_LOGI(TAG, "wifiEventHandler: WIFI_EVENT_STA_CONNECTED");
				// Connected, but still waiting for IP
				break;

			case WIFI_EVENT_STA_DISCONNECTED:
				ESP_LOGI(TAG, "wifiEventHandler: WIFI_EVENT_STA_DISCONNECTED");
				setState(WIFI_STATE_DISCONNECTED);
				disconnectHandler();
				break;

			case WIFI_EVENT_STA_BEACON_TIMEOUT:
				ESP_LOGI(TAG, "wifiEventHandler: WIFI_EVENT_STA_BEACON_TIMEOUT");
				disconnectHandler();
				break;
				
			default:
				ESP_LOGI(TAG, "wifiEventHandler: Unknown wifi event!");
				pWifiContext->bRecentError = true;
		}
	}
	
	// IP Events
	else if (event_base == IP_EVENT) {
		switch (event_id) {
			case IP_EVENT_STA_GOT_IP:
				ESP_LOGI(TAG, "wifiEventHandler: IP_EVENT_STA_GOT_IP");

				ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
				ESP_LOGI(TAG, "WiFi got ip: " IPSTR, IP2STR(&event->ip_info.ip));
				setState(WIFI_STATE_CONNECTED);
				pWifiContext->uRetryCount = 0;

				if (event->ip_changed) {
					// todo: Recreate sockets (reset MQTT?)
					MQTT_appStart();
				} else {
					// Connection complete
					MQTT_appStart();
				}
				break;
				
			case IP_EVENT_GOT_IP6:
				ESP_LOGI(TAG, "wifiEventHandler: IP_EVENT_GOT_IP6");
				break;
				
			case IP_EVENT_STA_LOST_IP:
				ESP_LOGI(TAG, "wifiEventHandler: IP_EVENT_STA_LOST_IP");
				break;
				
			default:
				ESP_LOGI(TAG, "wifiEventHandler: Unknown ip event!");
				pWifiContext->bRecentError = true;
		}
	}
}

static void setState(WIFI_STATE eState) {
	pWifiContext->eState = eState;
}

static void disconnectHandler() {
	if (pWifiContext->uRetryCount < WIFI_MAXIMUM_RETRY) {
		// Connection retry/recovery
		pWifiContext->uRetryCount++;
		ESP_LOGI(TAG, "WiFi connect failed, retry %i", pWifiContext->uRetryCount);
		esp_wifi_connect();
	} else {
		// Connection reset/failure
		ESP_LOGI(TAG, "WiFi connect failed, resetting netif");
		ESP_ERROR_CHECK(esp_wifi_stop());
		esp_netif_destroy_default_wifi(wifi_netif);
		wifi_netif = esp_netif_create_default_wifi_sta();
		pWifiContext->uRetryCount = 0;
		ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
		ESP_ERROR_CHECK(esp_wifi_start());
	}
}