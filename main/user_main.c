#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "hardware_monitor.h"
#include "hardware/wifi.h"

/******************************************************************************
 * FunctionName : app_main
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void app_main(void)
{
    hardware_monitor_init();

    wifi_init();

    while (1)
    {
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}
