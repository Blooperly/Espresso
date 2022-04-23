#pragma once

void ADMIN_watchdogInit();
void ADMIN_printAppHeader();
void ADMIN_printChipInfo();
void ADMIN_printFlashInfo();
void ADMIN_printHeapInfo();
void ADMIN_loggingTest();
void ADMIN_printTasks();

#define WATCHDOG_TIMEOUT 10