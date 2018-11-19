
#include "driver/gpio.h"
#include "driver/ht1623.h"

#define TAKE_CS() gpio_set_level(HT1623_CS_PIN, 0)
#define RELEASE_CS() gpio_set_level(HT1623_CS_PIN, 1)

void HT1621_writeBits(uint8_t data, uint8_t cnt)
{
    uint8_t i;

    for (i = 0; i < cnt; i++, data <<= 1)
    {
        gpio_set_level(HT1623_WR_PIN, 0);
        gpio_set_level(HT1623_DATA_PIN, data & 0x80 ? 1 : 0);
        gpio_set_level(HT1623_WR_PIN, 1);
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
    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = ((1ULL << HT1623_DATA_PIN) | (1ULL << HT1623_WR_PIN) | (1ULL << HT1623_CS_PIN));
    gpio_config(&io_conf);

    gpio_set_level(HT1623_CS_PIN, 1);
    gpio_set_level(HT1623_WR_PIN, 1);
    gpio_set_level(HT1623_DATA_PIN, 1);

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
    {
        HT1621_write(i, 0x00);
    }
}