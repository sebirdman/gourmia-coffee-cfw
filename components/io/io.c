#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "driver/mcp23s17.h"

#define IO_TAG "IO"
#define IO_TASK_SIZE 1024

typedef enum
{
    LED_TOP = 0,
    LED_BOTTOM,
    LED_SCREEN,
    UNKNOWN_ONEA,
    GRINDER,
    WATER_PUMP,
    WARMING_PAD,
    UNKOWN_TWOA,
} porta_t;

typedef enum
{
    BUTTON_ONE = 0,
    BUTTON_TWO,
    BUTTON_THREE,
    UNKNOWN_ONEB,
    UNKNOWN_TWOB,
    UNKNOWN_THREEB,
    BUTTON_BOTTOM,
    BUTTON_TOP,
} portb_t;

uint8_t _porta = 0b0000000;
uint8_t _portb = 0b0000000;
uint8_t _setGPIO = 0;

void toggle_led_top()
{
    _porta = _porta ^ (1UL << LED_TOP);
    _setGPIO = 1;
}

void toggle_led_bottom()
{
    _porta = _porta ^ (1UL << LED_BOTTOM);
    _setGPIO = 1;
}

void toggle_led_screen()
{
    _porta = _porta ^ (1UL << LED_BOTTOM);
    _setGPIO = 1;
}

void toggle_grinder()
{
    _porta = _porta ^ (1UL << GRINDER);
    _setGPIO = 1;
}

void toggle_water_pump()
{
    _porta = _porta ^ (1UL << WATER_PUMP);
    _setGPIO = 1;
}

void toggle_warming_pad()
{
    _porta = _porta ^ (1UL << WARMING_PAD);
    _setGPIO = 1;
}

void clear_all_outputs()
{
    _porta = 0b00000000;
    _setGPIO = 1;
}

uint8_t check_button_one()
{
    return _portb & (1UL << BUTTON_ONE);
}

uint8_t check_button_two()
{
    return _portb & (1UL << BUTTON_TWO);
}

uint8_t check_button_three()
{
    return _portb & (1UL << BUTTON_THREE);
}

uint8_t check_button_top()
{
    return _portb & (1UL << BUTTON_TOP);
}

uint8_t check_button_bottom()
{
    return _portb & (1UL << BUTTON_BOTTOM);
}

static void prv_io_task()
{
    mcp23s17_init();
    mcp23s17_REG_SET(IODIR_CTRL, PORTB, 0b11000111); // Top 3 bits and bottom 2 bits are inputs
    mcp23s17_REG_SET(IODIR_CTRL, PORTA, 0b10001000); //
    mcp23s17_REG_SET(GPPU_CTRL, PORTA, 0b01110111);  // Enable Output pull ups

    sGPIO_SET(PORTB, _portb); // All outputs OFF
    sGPIO_SET(PORTA, _porta); // All outputs OFF

    while (1)
    {
        toggle_led_top();

        _portb = sGPIO_READ(PORTB);
        if (check_button_one())
        {
            printf("Button one pressed\n");
        }
        if (check_button_two())
        {
            printf("Button two pressed\n");
        }
        if (check_button_three())
        {
            printf("Button three pressed\n");
        }
        if (check_button_top())
        {
            printf("Button top pressed\n");
        }
        if (check_button_bottom())
        {
            printf("Button top pressed\n");
        }

        if (_setGPIO)
        {
            sGPIO_SET(PORTA, _porta);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void io_init()
{
    xTaskCreate(&prv_io_task, IO_TAG, IO_TASK_SIZE, NULL, 7, NULL);
}