
#include "driver/ht1621.h"
#include "freertos/FreeRTOS.h"

#include "freertos/timers.h"

#include "hardware/display.h"
#include "hardware/io.h"

#include "hardware_monitor.h"

// we cannot use both the mcp23 and the ht1623 at the same time
// so we create this global semaphore to ensure we don't interfere with each other
SemaphoreHandle_t g_hardware_semaphore = NULL;

uint8_t making_coffee = 0;

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

static void prv_pot_full_callback(TimerHandle_t xTimer)
{
    //toggle_water_pump();
    printf("Turning off Water\n");
}

static void prv_start_filling_pot()
{
    //toggle_water_pump();
    xTimerCreate("Filling Pot Done",
                 ((1000 * 60) * 6) / portTICK_PERIOD_MS,
                 pdFALSE, (void *)0,
                 prv_pot_full_callback);
}

static void prv_warm_plate_done_callback(TimerHandle_t xTimer)
{
    //toggle_warming_pad();
    printf("Turning off Plate\n");

    making_coffee = 0;
}

static void prv_start_warm_pad()
{
    //toggle_warming_pad();
    xTimerCreate("Warm Pad Done",
                 ((1000 * 60) * 60) / portTICK_PERIOD_MS,
                 pdFALSE, (void *)0,
                 prv_warm_plate_done_callback);
}

static void prv_grind_finished_callback(TimerHandle_t xTimer)
{
    //toggle_grinder();
    printf("Finished grinding");
    prv_start_warm_pad();
    prv_start_filling_pot();
}

static void prv_start_grinder()
{
    //toggle_grinder();
    xTimerCreate("Grinder Done",
                 (1000 * 15) / portTICK_PERIOD_MS,
                 pdFALSE, (void *)0,
                 prv_grind_finished_callback);
}

void hardware_start_coffee()
{
    if (!making_coffee)
    {
        prv_start_warm_pad();
        prv_start_filling_pot();
        making_coffee = 1;
    }
}