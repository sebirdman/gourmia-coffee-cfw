#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "hardware/display.h"
#include "hardware/io.h"


/******************************************************************************
 * FunctionName : app_main
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void app_main(void)
{
    display_init();
    io_init();

    while (1)
    {
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}
