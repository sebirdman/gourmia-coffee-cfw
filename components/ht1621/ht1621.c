#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#include "driver/gpio.h"
#include "driver/ht1621.h"

#define prv_take_cs() gpio_set_level(HT1623_CS_PIN, 0)
#define prv_release_cs() gpio_set_level(HT1623_CS_PIN, 1)

struct ht1621_mutex_interface _prv_interface;

static void prepare_gpio() {
    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = ((1ULL << HT1623_DATA_PIN) | (1ULL << HT1623_WR_PIN) | (1ULL << HT1623_CS_PIN));
    gpio_config(&io_conf);
}

static void prv_ht1621_write_bits(uint8_t data, uint8_t cnt)
{
    uint8_t i;
    for (i = 0; i < cnt; i++, data <<= 1)
    {
        gpio_set_level(HT1623_WR_PIN, 0);
        gpio_set_level(HT1623_DATA_PIN, data & 0x80 ? 1 : 0);
        gpio_set_level(HT1623_WR_PIN, 1);
    }
}

static void prv_ht1621_send_command(uint8_t cmd)
{
    _prv_interface.take();

    prv_take_cs();
    prv_ht1621_write_bits(COMMAND_MODE, 4);
    prv_ht1621_write_bits(cmd, 8);
    prv_release_cs();

    _prv_interface.release();
}


void HT1621_write(uint8_t address, uint8_t data)
{
    if (address > 7)
    {
        return;
    }

    _prv_interface.take();
    prepare_gpio();
    prv_take_cs();

    prv_ht1621_write_bits(WRITE_MODE, 3);
    prv_ht1621_write_bits(address << 2, 6); // 6 is because max address is 128
    prv_ht1621_write_bits(data << 4, 4);    // 6 is because max address is 128

    prv_release_cs();
    _prv_interface.release();
}

void HT1621_init(struct ht1621_mutex_interface interface)
{
    _prv_interface = interface;
    prepare_gpio();
    prv_release_cs();

    prv_ht1621_send_command(BIAS_THIRD_4_COM);
    prv_ht1621_send_command(RC256K);

    prv_ht1621_send_command(SYS_DIS);
    prv_ht1621_send_command(WDT_DIS);

    prv_ht1621_send_command(SYS_EN);
    prv_ht1621_send_command(LCD_ON);
}

void HT1621_clear()
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        HT1621_write(i, 0x00);
    }
}