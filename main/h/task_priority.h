#pragma once

// Priority 0-24 (higher number = higher priority)
//		24 = ipc tasks
//		1 = main task
// 		0 = idle task

#define TASK_PRIORITY_MQTT		5	// Set in menuconfig
#define TASK_PRIORITY_APP		4
#define TASK_PRIORITY_HT16K33	4