#include <stdint.h>

#include "config.h"
#include "doomgeneric.h"
#include "gpio_input.h"
#include "lcd_init.h"
#include "sd_driver.h"

#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"

void TM4C_SetSystemClockHz(uint32_t system_clock_hz);

int main(void)
{
    uint32_t system_clock_hz;
    static char wad_path[] = TM4C_WAD_PATH;
    static char zone_megabytes[] = "16";
    static char *doom_argv[] = {
        "tm4c-doom",
        "-iwad",
        wad_path,
        "-mb",
        zone_megabytes,
        "-nosound",
        NULL,
    };

    system_clock_hz = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                          SYSCTL_OSC_MAIN |
                                          SYSCTL_USE_PLL |
                                          SYSCTL_CFG_VCO_480),
                                         TM4C_SYSTEM_CLOCK_HZ);

    TM4C_SetSystemClockHz(system_clock_hz);

    LCD_Init();
    GPIO_InputInit();

    if (!SD_DriverInit() || !SD_DriverMount())
    {
        LCD_Clear(0xF800U);
        IntMasterDisable();
        while (1)
        {
        }
    }

    doomgeneric_Create(6, doom_argv);

    while (1)
    {
        doomgeneric_Tick();
    }
}
