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

// Defines
#define EXAMPLE_ESP_WIFI_SSID      "The Peoples Coffeehouse"
#define EXAMPLE_ESP_WIFI_PASS      "Haymarket1886"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

// Local Variables
static const char *TAG = "WIFI";

typedef struct {
	WIFI_STATE eState;
} WIFI_CONTEXT;

static WIFI_CONTEXT sWifiContext;
static WIFI_CONTEXT* pWifiContext;
static int s_retry_num = 0;

// Local Function Definitions
static void wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void setState(WIFI_STATE eState);

// Module Functions
void WIFI_init(void) {
	// Initialize Wifi Context Pointer
	pWifiContext = &sWifiContext;

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
	
	// Initialize Wifi
    ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;	// *J* Figure out how event registration works
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiEventHandler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiEventHandler, NULL, &instance_got_ip));

	// Wifi Connection Parameters
	// todo: Handle WiFi configuration over bluetooth?
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
			.scan_method = WIFI_FAST_SCAN,
	     	.threshold.authmode = WIFI_AUTH_WPA2_PSK, 	// Arch: Remove this to enable connection to less secure AP's

            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };

	// Run Wifi
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

	uint8_t wifi_mac[6];
	ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, wifi_mac));
	ESP_LOGI(TAG, "Starting WiFi for %02X:%02X:%02X:%02X:%02X:%02X", wifi_mac[0], wifi_mac[1], wifi_mac[2], wifi_mac[3], wifi_mac[4], wifi_mac[5]);

    ESP_ERROR_CHECK(esp_wifi_start());
}

WIFI_STATE WIFI_getState() {
	return pWifiContext->eState;
}

// Local Functions
static void wifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            //xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        //xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
		mqtt_app_start();
    }
}

static void setState(WIFI_STATE eState) {
	pWifiContext->eState = eState;
}
