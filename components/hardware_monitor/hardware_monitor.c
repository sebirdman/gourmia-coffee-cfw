
#include "driver/ht1621.h"

#include "hardware/display.h"
#include "hardware/io.h"

#include "hardware_monitor.h"

// we cannot use both the mcp23 and the ht1623 at the same time
// so we create this global semaphore to ensure we don't interfere with each other
SemaphoreHandle_t g_hardware_semaphore = NULL;

static void prv_take()
{
    while (xSemaphoreTake(g_hardware_semaphore, portMAX_DELAY) != pdTRUE)
        ;
}

static void prv_release()
{
    xSemaphoreGive(g_hardware_semaphore);
}

void hardware_monitor_init()
{
    g_hardware_semaphore = xSemaphoreCreateMutex();

    struct ht1621_mutex_interface display_interface = {
        .take = prv_take,
        .release = prv_release,
    };

    struct mcp23s17_mutex_interface io_interface = {
        .take = prv_take,
        .release = prv_release,
    };

    display_init(display_interface);
    io_init(io_interface);
}