/*
* The MIT License (MIT)
* 
* Copyright (c) 2015 David Ogilvy (MetalPhreak)
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef MCP23S17_H
#define MCP23S17_H

#include "driver/spi.h"
#include "driver/mcp23s17_register.h"

struct mcp23s17_mutex_interface
{
    void (*take)();
    void (*release)();
};

void mcp23s17_init(struct mcp23s17_mutex_interface io_interface);
void mcp23s17_REG_SET(uint8_t ctrl_reg, uint8_t port, uint16_t value);
uint16_t mcp23s17_REG_GET(uint8_t ctrl_reg, uint8_t port);
void mcp23s17_REG_SET_MASK(uint8_t ctrl_reg, uint8_t port, uint16_t value, uint16_t bitmask);

#define sGPIO_SET(port, value) mcp23s17_REG_SET(OLAT_CTRL, port, value)
#define sGPIO_SET_MASK(port, value, bitmask) mcp23s17_REG_SET_MASK(OLAT_CTRL, port, value, bitmask)
#define sGPIO_SET_PIN(port, pin, value) mcp23s17_REG_SET_MASK(OLAT_CTRL, port, value<<(pin-1), 1<<(pin-1))

#define sGPIO_GET(port) mcp23s17_REG_GET(OLAT_CTRL, port)

#define sGPIO_READ(port) mcp23s17_REG_GET(GPIO_CTRL, port)






#endif

