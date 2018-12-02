#ifndef IO_H
#define IO_H

#include <stdint.h>
#include "driver/mcp23s17.h"

void io_init(struct mcp23s17_mutex_interface interface);

void toggle_led_top();
void toggle_led_bottom();
void toggle_led_screen();
void toggle_grinder();
void toggle_water_pump();
void toggle_warming_pad();

uint8_t is_button_top_pressed();
uint8_t is_button_bottom_pressed();
uint8_t is_button_three_pressed();
uint8_t is_button_two_pressed();
uint8_t is_button_one_pressed();

#endif /* IO_H */