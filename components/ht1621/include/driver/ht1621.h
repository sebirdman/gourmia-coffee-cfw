
#ifndef HT1623_H
#define HT1623_H

#include <stdint.h>

struct ht1621_mutex_interface
{
    void (*take)();
    void (*release)();
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
    TONE_ON = 0x12,  /*!< Start emitting tone signal at the tone pin. Tone frequency is selected using commands TONE2K or TONE4K. \sa TONE2K, TONE4K */
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

#define HT1623_DATA_PIN GPIO_NUM_13
#define HT1623_WR_PIN GPIO_NUM_14
#define HT1623_CS_PIN GPIO_NUM_4

void HT1621_init(struct ht1621_mutex_interface interface);
void HT1621_clear();
void HT1621_write(uint8_t address, uint8_t data);

#endif /* HT1623_H */