// Logging
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

// Includes
#include <stdio.h>
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_spi_flash.h"
#include "esp_chip_info.h"
#include "sdkconfig.h"
#include "esp_task_wdt.h"

// Project Includes
#include "h\admin.h"

static const char* TAG = "ADMIN";

void ADMIN_watchdogInit() {
    ESP_LOGI(TAG, "ADMIN_watchdogInit()");

    // Interrupt Watchdog Timer
        // ESP-IDF includes an interrupt watchdog timer which checks that the FreeRTOS tick
        // interrupt is not blocked by functions which disable interrupts for too long. This
        // is enabled and configured through the ESP IDF config menu.

    // Task Watchdog Timer
        // The task watchdog timer (TWDT) is different than the interrupt watchdog timer. The
        // TWDT ensures that tasks are run frequently enough, and not blocked by CPU hungry
        // tasks with a higher priority. By default, the TWDT only makes sure that each core's
        // idle task runs within the watchdog's timout.

    ESP_ERROR_CHECK(
        esp_task_wdt_init(WATCHDOG_TIMEOUT, true)
    );
}

void ADMIN_printAppHeader() {
    printf("\n\n\n\
        #####################################################################################\n\
        #                                    Espresso                                       #\n\
        #                                     v0.0.1                                        #\n\
        #####################################################################################\n\n\n");
    
    ADMIN_printChipInfo();
    ADMIN_printFlashInfo();
    ADMIN_printHeapInfo();
    ADMIN_loggingTest();
}

void ADMIN_printChipInfo() {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    printf(" CHIP INFO\n =========\n");
    printf(" Model: %s\n", CONFIG_IDF_TARGET);
    printf(" Chip Features:%s%s%s%s\n",
        (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? " WiFi" : "",
        (chip_info.features & CHIP_FEATURE_BT) ? " BT" : "",
        (chip_info.features & CHIP_FEATURE_BLE) ? " BLE" : "",
        (chip_info.features & CHIP_FEATURE_IEEE802154) ? " IEEE802154" : "");
    printf(" Cores: %i\n", chip_info.cores);
    printf(" Revision: %i\n", chip_info.revision);
    printf("\n");
}

void ADMIN_printFlashInfo() {
    printf(" FLASH INFO\n ==========\n");
    printf(" Flash size: %s (%u bytes)\n", CONFIG_ESPTOOLPY_FLASHSIZE, spi_flash_get_chip_size());
    printf("\n");
}

void ADMIN_printHeapInfo() {
    multi_heap_info_t heap_info;
    heap_caps_get_info(&heap_info, MALLOC_CAP_8BIT);

    printf(" HEAP INFO\n =========\n");
    printf(" Heap size: %u\n", heap_info.total_allocated_bytes);
    printf(" Heap free: %u\n", heap_info.total_free_bytes);
    printf(" Heap blocks (allocated/free/total): (%u/%u/%u)\n", heap_info.allocated_blocks, heap_info.free_blocks, heap_info.total_blocks);
    printf(" Largest free block: %u\n", heap_info.largest_free_block);
    printf("\n");
}

void ADMIN_loggingTest() {
    ESP_LOGE(TAG, "Logging test: Error");
    ESP_LOGW(TAG, "Logging test: Warning");
    ESP_LOGI(TAG, "Logging test: Info");
    ESP_LOGD(TAG, "Logging test: Debug");
    ESP_LOGV(TAG, "Logging test: Verbose");
    printf("\n");
}

void ADMIN_printTasks() {
    printf("\n");
    char task_buffer[700];
    vTaskList(task_buffer);
    printf("%s", task_buffer);
    printf("\n");
}