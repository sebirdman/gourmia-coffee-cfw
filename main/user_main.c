/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>
#include <stdint.h>
#include "driver/gpio.h"

#include "driver/mcp23s17.h"
#include "driver/ht1623.h"

void setup_screen()
{
    HT1621_init();
    HT1621_clear();
    for (uint8_t i = 0; i < 32; i++)
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

void setup_expander()
{
    mcp23s17_init();
    mcp23s17_REG_SET(IODIR_CTRL, PORTB, 0b11000111); // Top 3 bits and bottom 2 bits are inputs
    mcp23s17_REG_SET(IODIR_CTRL, PORTA, 0b10001000); //
    mcp23s17_REG_SET(GPPU_CTRL, PORTA, 0b01110111);  // Enable Output pull ups
    sGPIO_SET(PORTB, 0x0000);                        // All outputs OFF
    sGPIO_SET(PORTA, 0x0000);                        // All outputs OFF
}

static void printer(uint16_t n)
{
    int c, k;
    for (c = 31; c >= 0; c--)
    {
        k = n >> c;

        if (k & 1)
            printf("1");
        else
            printf("0");
    }
}

uint16_t readGPIO(void)
{
    return sGPIO_READ(PORTA);
}

uint16_t readOLAT(void)
{
    return sGPIO_GET(PORTA);
}

void printIOreg(void)
{
    printf("OLAT = ");
    printer(readOLAT());
    printf("GPIO = ");
    printer(readGPIO());
    printf("IODIR = %02x\n", mcp23s17_REG_GET(IODIR_CTRL, PORTA));
}

/******************************************************************************
 * FunctionName : app_main
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void app_main(void)
{
    printf("SDK version:%s\n", esp_get_idf_version());
    setup_screen();
    setup_expander();

    while (1)
    {
        printIOreg();
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}
