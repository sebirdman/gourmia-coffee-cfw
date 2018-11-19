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

int _DATA_pin;
int _RW_pin;
int _CS_pin;

#define DATA 0
#define RW 1
#define CS 2

#define TAKE_CS() gpio_set_level(_CS_pin, 0)
#define RELEASE_CS() gpio_set_level(_CS_pin, 1)

uint8_t pattern[] = {
    0xfa, // 0
    0x0a, // 1
    0xbc, // 2
    0x9e, // 3
    0x4e, // 4
    0xd6, // 5
    0xf6, // 6
    0x8a, // 7
    0xfe, // 8
    0xde, // 9
    0x04, // -
    0x10, // _
    0x6e, // H
    0xf4, // E
    0x70, // L
    0xec, // P
    0x00  // Space
};

enum Commands
{
    SYS_DIS = 0x00,   /*!< System disable. It stops the bias generator and the system oscillator. */
    SYS_EN = 0x02,    /*!< System enable. It starts the bias generator and the system oscillator. */
    LCD_OFF = 0x04,   /*!< Turn off the bias generator. */
    LCD_ON = 0x06,    /*!< Turn on the bias generator. */
    TIMER_DIS = 0x08, /*!< Disable time base output. */
    WDT_DIS = 0x0a,   /*!< Watch-dog timer disable. */
    TIMER_EN = 0x0c,  /*!< Enable time base output. */
    WDT_EN = 0x0e,    /*!< Watch-dog timer enable. The timer is reset. */
    CLR_TIMER = 0x18, /*!< Clear the contents of the time base generator. */
    CLR_WDT = 0x1c,   /*!< Clear the contents of the watch-dog stage. */

    TONE_OFF = 0x10, /*!< Stop emitting the tone signal at the tone pin. \sa TONE2K, TONE4K */
    TONE_ON = 0x12,  /*!< Start emitting tone signal at the tone pin. Tone frequency
is selected using commands TONE2K or TONE4K. \sa TONE2K, TONE4K */
    TONE2K = 0xc0,   /*!< Output tone is at 2kHz. */
    TONE4K = 0x80,   /*!< Output tone is at 4kHz. */

    RC256K = 0x30,  /*!< System oscillator is the internal RC oscillator at 256kHz.  */
    XTAL32K = 0x50, /*!< System oscillator is the crystal oscillator at 32768Hz. */
    EXT256K = 0x38, /*!< System oscillator is an external oscillator at 256kHz. */

    //Set bias to 1/2 or 1/3 cycle
    //Set to 2,3 or 4 connected COM lines
    BIAS_HALF_2_COM = 0x40,  /*!< Use 1/2 bias and 2 commons. */
    BIAS_HALF_3_COM = 0x48,  /*!< Use 1/2 bias and 3 commons. */
    BIAS_HALF_4_COM = 0x50,  /*!< Use 1/2 bias and 4 commons. */
    BIAS_THIRD_2_COM = 0x42, /*!< Use 1/3 bias and 2 commons. */
    BIAS_THIRD_3_COM = 0x4a, /*!< Use 1/3 bias and 3 commons. */
    BIAS_THIRD_4_COM = 0x52, /*!< Use 1/3 bias and 4 commons. */

    IRQ_EN = 0x10,  /*!< Enables IRQ output. This needs to be excuted in SPECIAL_MOD E. */
    IRQ_DIS = 0x10, /*!< Disables IRQ output. This needs to be excuted in SPECIAL_MO DE. */

    // WDT configuration commands
    F1 = 0x80,   /*!< Time base/WDT clock. Output = 1Hz. Time-out = 4s. This needs to be excuted in SPECIAL_MODE. */
    F2 = 0x42,   /*!< Time base/WDT clock. Output = 2Hz. Time-out = 2s. This needs to be excuted in SPECIAL_MODE. */
    F4 = 0x44,   /*!< Time base/WDT clock. Output = 4Hz. Time-out = 1s. This needs to be excuted in SPECIAL_MODE. */
    F8 = 0x46,   /*!< Time base/WDT clock. Output = 8Hz. Time-out = .5s. This needs to be excuted in SPECIAL_MODE. */
    F16 = 0x48,  /*!< Time base/WDT clock. Output = 16Hz. Time-out = .25s. This needs to be excuted in SPECIAL_MODE. */
    F32 = 0x4a,  /*!< Time base/WDT clock. Output = 32Hz. Time-out = .125s. This needs to be excuted in SPECIAL_MODE. */
    F64 = 0x4c,  /*!< Time base/WDT clock. Output = 64Hz. Time-out = .0625s. This needs to be excuted in SPECIAL_MODE. */
    F128 = 0x4e, /*!< Time base/WDT clock. Output = 128Hz. Time-out = .03125s. This needs to be excuted in SPECIAL_MODE. */

    //Don't use
    TEST_ON = 0xc0,  /*!< Don't use! Only for manifacturers. This needs SPECIAL_MODE. */
    TEST_OFF = 0xc6, /*!< Don't use! Only for manifacturers. This needs SPECIAL_MODE. */

    COMMAND_MODE = 0x80,           /*!< This is used for sending standard commands. */
    READ_MODE = 0xc0,              /*!< This instructs the HT1621 to prepare for reading the internal RAM. */
    WRITE_MODE = 0xa0,             /*!< This instructs the HT1621 to prepare for writing the internal RAM. */
    READ_MODIFY_WRITE_MODE = 0xa0, /*!< This instructs the HT1621 to prepare for reading/modifying batch of internal RAM adresses. */
    SPECIAL_MODE = 0x90            /*!< This instructs the HT1621 to prepare for executing a special command. */

};

void HT1621_begin(int cs, int rw, int data)
{
    _DATA_pin = data;
    _RW_pin = rw;
    _CS_pin = cs;

    gpio_config_t io_conf;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO15/16
    io_conf.pin_bit_mask = ((1ULL << _DATA_pin) | (1ULL << _RW_pin) | (1ULL << _CS_pin));
    //configure GPIO with the given settings
    gpio_config(&io_conf);
}

void HT1621_writeBits(uint8_t data, uint8_t cnt)
{
    uint8_t i;

    for (i = 0; i < cnt; i++, data <<= 1)
    {
        gpio_set_level(_RW_pin, 0);
        gpio_set_level(_DATA_pin, data & 0x80 ? 1 : 0);
        gpio_set_level(_RW_pin, 1);
    }
}

void HT1621_write(uint8_t address, uint8_t data)
{
    TAKE_CS();
    HT1621_writeBits(WRITE_MODE, 3);
    HT1621_writeBits(address << 2, 6); // 6 is because max address is 128
    HT1621_writeBits(data << 4, 4);    // 6 is because max address is 128
    RELEASE_CS();
}

void HT1621_sendCommand(uint8_t cmd)
{
    TAKE_CS();
    HT1621_writeBits(COMMAND_MODE, 4);
    HT1621_writeBits(cmd, 8);
    RELEASE_CS();
}

void HT1621_init()
{
    gpio_set_level(_CS_pin, 1);
    gpio_set_level(_RW_pin, 1);
    gpio_set_level(_DATA_pin, 1);

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    HT1621_sendCommand(BIAS_THIRD_4_COM);
    HT1621_sendCommand(RC256K);

    HT1621_sendCommand(SYS_DIS);
    HT1621_sendCommand(WDT_DIS);

    HT1621_sendCommand(SYS_EN);
    HT1621_sendCommand(LCD_ON);
}

void HT1621_clear()
{
    uint8_t i;
    for (i = 0; i < 32; i++)
        HT1621_write(i, 0x00);
}

void HT1621_setNum(uint8_t adr, uint8_t num)
{
    HT1621_write(adr, pattern[num]);
}

void HT1621_setDotNum(uint8_t adr, uint8_t num)
{
    HT1621_write(adr, pattern[num] | 0x01);
}

void setup_screen()
{
    HT1621_begin(GPIO_NUM_4, GPIO_NUM_14, GPIO_NUM_13);
    HT1621_init();
    // clear memory

    // all digit ON
    HT1621_clear();
    for (uint8_t i = 0; i < 32; i++)
    {
        uint8_t t = 0x01;
        for (uint8_t j = 0; j < 4; j++)
        {
            HT1621_write(i, t);
            t <<= 1;
            t++;
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
    }
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
    mcp23s17_init();

	//Enable hardware addressing & sequential addressing on all devices
	// mcp23s17_REG_SET(IOCON_CTRL, PORTA, SEQOP | HAEN);

    mcp23s17_REG_SET(IODIR_CTRL,    PORTB,  0b11000111); // Top 3 bits and bottom 2 bits are inputs
    mcp23s17_REG_SET(IODIR_CTRL,    PORTA,  0b10001000); //
    mcp23s17_REG_SET(GPPU_CTRL,     PORTA,  0b01110111);  // Enable Output pull ups
    sGPIO_SET(PORTB, 0x00); // All outputs OFF
    sGPIO_SET(PORTA, 0b00100111);

    while (1)
    {
        printIOreg();
        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}
