#ifndef HARDWARE_MONITOR_H
#define HARDWARE_MONITOR_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

SemaphoreHandle_t g_hardware_semaphore;

void hardware_monitor_init();
void hardware_start_coffee();

#endif /* HARDWARE_MONITOR_H */