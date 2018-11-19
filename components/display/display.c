#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "driver/ht1623.h"

#define DISPLAY_TAG "DISPLAY"

#define DISPLAY_TASK_SIZE 1024

static void prv_test()
{
    for (uint8_t i = 5; i < 7; i++)
    {
        uint8_t t = 0x01;
        for (uint8_t j = 0; j < 4; j++)
        {
            HT1621_write(i, t);
            t <<= 1;
            t++;
        }
    }
}

void prv_display_task()
{
    HT1621_init();
    HT1621_clear();
    prv_test();

    while (1)
    {
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}

void display_init()
{
    xTaskCreate(&prv_display_task, DISPLAY_TAG, DISPLAY_TASK_SIZE, NULL, 7, NULL);
}
