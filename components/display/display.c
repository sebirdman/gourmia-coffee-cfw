#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <time.h>

#include "hardware/io.h"
#include "driver/ht1621.h"

#define DISPLAY_TAG "DISPLAY"

#define DISPLAY_TASK_SIZE 1024 * 2

#define STATE_ONE (1UL << 0)
#define STATE_TWO (1UL << 1)
#define STATE_THREE (1UL << 2)
#define STATE_FOUR (1UL << 3)

struct ht1621_mutex_interface _prv_interface;

enum address_zero_two_four
{
    TOP_RIGHT = STATE_ONE,
    MIDDLE = STATE_TWO,
    BOTTOM_RIGHT = STATE_THREE,
};

enum address_one_three_five
{
    TOP = STATE_ONE,
    TOP_LEFT = STATE_TWO,
    BOTTOM_LEFT = STATE_THREE,
    BOTTOM = STATE_FOUR,
};

enum address_zero
{
    CLOCK = STATE_FOUR,
};

enum address_two
{
    COLON = STATE_FOUR,
};

enum address_four
{
    UNKNOWN1 = STATE_FOUR,
};

enum address_six
{
    TIMER = STATE_ONE,
    ONE_TOP_RIGHT = STATE_TWO,
    ONE_BOTTOM_RIGHT = STATE_THREE,
    UNKNOWN2 = STATE_FOUR,
};

enum address_seven
{
    BEAN = STATE_ONE,
    CUP = STATE_TWO,
    ONE_NUMBER_TWO = STATE_THREE, // this is a bizzare one, basically this plus ONE_TOP_RIGHT will get you the number two on digit one
    UNKNOWN3 = STATE_FOUR,
};

uint8_t bean_enabled = 0;
uint8_t mug_enabled = 0;
uint8_t timer_enabled = 0;
uint8_t clock_enabled = 0;
uint8_t colon_enabled = 0;

static uint8_t set_register_zero_with_clock(uint8_t desired_add)
{
    return clock_enabled ? CLOCK | desired_add : desired_add;
}

static uint8_t set_register_six_with_timer(uint8_t desired_add)
{
    return timer_enabled ? TIMER | desired_add : desired_add;
}

static uint8_t set_register_two_with_colon(uint8_t desired_add)
{
    return colon_enabled ? COLON | desired_add : desired_add;
}

static void draw_digit_four(uint8_t num)
{

    if (num > 2)
    {
        return;
    }

    uint8_t add_item = mug_enabled + bean_enabled;
    uint8_t item_to_add = mug_enabled ? CUP : 0x00;
    item_to_add = bean_enabled ? BEAN | item_to_add : item_to_add;

    switch (num)
    {
    case 0:
        HT1621_write(6, set_register_six_with_timer(0x00));
        HT1621_write(7, add_item ? item_to_add : 0x00);
        break;
    case 1:
        HT1621_write(6, set_register_six_with_timer(ONE_TOP_RIGHT | ONE_BOTTOM_RIGHT));
        HT1621_write(7, add_item ? item_to_add : 0x00);
        break;
    case 2:
        HT1621_write(6, set_register_six_with_timer(ONE_TOP_RIGHT));
        HT1621_write(7, add_item ? ONE_NUMBER_TWO | item_to_add : ONE_NUMBER_TWO);
        break;
    }
}

static void draw_digit_three(uint8_t num)
{
    if (num > 9)
    {
        return;
    }

    switch (num)
    {
    case 0:
        HT1621_write(4, TOP_RIGHT | BOTTOM_RIGHT);
        HT1621_write(5, BOTTOM | TOP | TOP_LEFT | BOTTOM_LEFT);
        break;
    case 1:
        HT1621_write(4, TOP_RIGHT | BOTTOM_RIGHT);
        HT1621_write(5, 0x00);
        break;
    case 2:
        HT1621_write(4, TOP_RIGHT | MIDDLE);
        HT1621_write(5, TOP | BOTTOM_LEFT | BOTTOM);
        break;
    case 3:
        HT1621_write(4, TOP_RIGHT | MIDDLE | BOTTOM_RIGHT);
        HT1621_write(5, TOP | BOTTOM);
        break;
    case 4:
        HT1621_write(4, TOP_RIGHT | MIDDLE | BOTTOM_RIGHT);
        HT1621_write(5, TOP_LEFT);
        break;
    case 5:
        HT1621_write(4, BOTTOM_RIGHT | MIDDLE);
        HT1621_write(5, TOP | TOP_LEFT | BOTTOM);
        break;
    case 6:
        HT1621_write(4, BOTTOM_RIGHT | MIDDLE);
        HT1621_write(5, TOP | TOP_LEFT | BOTTOM | BOTTOM_LEFT);
        break;
    case 7:
        HT1621_write(4, TOP_RIGHT | BOTTOM_RIGHT);
        HT1621_write(5, TOP);
        break;
    case 8:
        HT1621_write(4, BOTTOM_RIGHT | MIDDLE | TOP_RIGHT);
        HT1621_write(5, TOP | TOP_LEFT | BOTTOM | BOTTOM_LEFT);
        break;
    case 9:
        HT1621_write(4, BOTTOM_RIGHT | MIDDLE | TOP_RIGHT);
        HT1621_write(5, TOP | TOP_LEFT | BOTTOM);
        break;
    default:
        break;
    }
}

static void draw_digit_two(uint8_t num)
{
    if (num > 9)
    {
        return;
    }

    switch (num)
    {
    case 0:
        HT1621_write(2, set_register_two_with_colon(TOP_RIGHT | BOTTOM_RIGHT));
        HT1621_write(3, BOTTOM | TOP | TOP_LEFT | BOTTOM_LEFT);
        break;
    case 1:
        HT1621_write(2, set_register_two_with_colon(TOP_RIGHT | BOTTOM_RIGHT));
        HT1621_write(3, 0x00);
        break;
    case 2:
        HT1621_write(2, set_register_two_with_colon(TOP_RIGHT | MIDDLE));
        HT1621_write(3, TOP | BOTTOM_LEFT | BOTTOM);
        break;
    case 3:
        HT1621_write(2, set_register_two_with_colon(TOP_RIGHT | MIDDLE | BOTTOM_RIGHT));
        HT1621_write(3, TOP | BOTTOM);
        break;
    case 4:
        HT1621_write(2, set_register_two_with_colon(TOP_RIGHT | MIDDLE | BOTTOM_RIGHT));
        HT1621_write(3, TOP_LEFT);
        break;
    case 5:
        HT1621_write(2, set_register_two_with_colon(BOTTOM_RIGHT | MIDDLE));
        HT1621_write(3, TOP | TOP_LEFT | BOTTOM);
        break;
    case 6:
        HT1621_write(2, set_register_two_with_colon(BOTTOM_RIGHT | MIDDLE));
        HT1621_write(3, TOP | TOP_LEFT | BOTTOM | BOTTOM_LEFT);
        break;
    case 7:
        HT1621_write(2, set_register_two_with_colon(TOP_RIGHT | BOTTOM_RIGHT));
        HT1621_write(3, TOP);
        break;
    case 8:
        HT1621_write(2, set_register_two_with_colon(BOTTOM_RIGHT | MIDDLE | TOP_RIGHT));
        HT1621_write(3, TOP | TOP_LEFT | BOTTOM | BOTTOM_LEFT);
        break;
    case 9:
        HT1621_write(2, set_register_two_with_colon(BOTTOM_RIGHT | MIDDLE | TOP_RIGHT));
        HT1621_write(3, TOP | TOP_LEFT | BOTTOM);
        break;
    default:
        break;
    }
}

static void draw_digit_one(uint8_t num)
{
    if (num > 9)
    {
        return;
    }

    switch (num)
    {
    case 0:
        HT1621_write(0, set_register_zero_with_clock(TOP_RIGHT | BOTTOM_RIGHT));
        HT1621_write(1, BOTTOM | TOP | TOP_LEFT | BOTTOM_LEFT);
        break;
    case 1:
        HT1621_write(0, set_register_zero_with_clock(TOP_RIGHT | BOTTOM_RIGHT));
        HT1621_write(1, 0x00);
        break;
    case 2:
        HT1621_write(0, set_register_zero_with_clock(TOP_RIGHT | MIDDLE));
        HT1621_write(1, TOP | BOTTOM_LEFT | BOTTOM);
        break;
    case 3:
        HT1621_write(0, set_register_zero_with_clock(TOP_RIGHT | MIDDLE | BOTTOM_RIGHT));
        HT1621_write(1, TOP | BOTTOM);
        break;
    case 4:
        HT1621_write(0, set_register_zero_with_clock(TOP_RIGHT | MIDDLE | BOTTOM_RIGHT));
        HT1621_write(1, TOP_LEFT);
        break;
    case 5:
        HT1621_write(0, set_register_zero_with_clock(BOTTOM_RIGHT | MIDDLE));
        HT1621_write(1, TOP | TOP_LEFT | BOTTOM);
        break;
    case 6:
        HT1621_write(0, set_register_zero_with_clock(BOTTOM_RIGHT | MIDDLE));
        HT1621_write(1, TOP | TOP_LEFT | BOTTOM | BOTTOM_LEFT);
        break;
    case 7:
        HT1621_write(0, set_register_zero_with_clock(TOP_RIGHT | BOTTOM_RIGHT));
        HT1621_write(1, TOP);
        break;
    case 8:
        HT1621_write(0, set_register_zero_with_clock(BOTTOM_RIGHT | MIDDLE | TOP_RIGHT));
        HT1621_write(1, TOP | TOP_LEFT | BOTTOM | BOTTOM_LEFT);
        break;
    case 9:
        HT1621_write(0, set_register_zero_with_clock(BOTTOM_RIGHT | MIDDLE | TOP_RIGHT));
        HT1621_write(1, TOP | TOP_LEFT | BOTTOM);
        break;
    default:
        break;
    }
}

static void draw_screen(uint16_t desired_number)
{
    if (desired_number > 2999)
    {
        return;
    }
    div_t thousands_result = div(desired_number, 1000);
    draw_digit_four(thousands_result.quot);
    div_t hundreds_result = div(thousands_result.rem, 100);
    draw_digit_three(hundreds_result.quot);
    div_t tens_result = div(hundreds_result.rem, 10);
    draw_digit_two(tens_result.quot);
    draw_digit_one(tens_result.rem);
}

void display_toggle_bean()
{
    bean_enabled = bean_enabled ? 0 : 1;
}

void display_toggle_mug()
{
    mug_enabled = mug_enabled ? 0 : 1;
}

void display_toggle_timer()
{
    timer_enabled = timer_enabled ? 0 : 1;
}

void display_toggle_clock()
{
    clock_enabled = clock_enabled ? 0 : 1;
}

void display_toggle_colon()
{
    colon_enabled = colon_enabled ? 0 : 1;
}

void prv_display_task()
{
    HT1621_init(_prv_interface);
    HT1621_clear();

    toggle_led_screen();
    display_toggle_timer();
    display_toggle_mug();
    display_toggle_bean();
    display_toggle_clock();
    uint16_t i = 0;
    while (1)
    {
        time_t now = 0;
        struct tm timeinfo = {0};

        time(&now);
        localtime_r(&now, &timeinfo);

        if (timeinfo.tm_hour == 0 && timeinfo.tm_min == 0)
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        div_t hour = div(timeinfo.tm_hour, 10);
        draw_digit_four(hour.quot);
        draw_digit_three(hour.rem);

        div_t minutes = div(timeinfo.tm_min, 10);
        draw_digit_two(minutes.quot);
        draw_digit_one(minutes.rem);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        display_toggle_colon();
        i++;
    }
}

void display_init(struct ht1621_mutex_interface interface)
{
    _prv_interface = interface;
    xTaskCreate(&prv_display_task, DISPLAY_TAG, DISPLAY_TASK_SIZE, NULL, 7, NULL);
}
