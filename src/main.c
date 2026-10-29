#include <stdint.h>

#include "driverlib/fpu.h"
#include "config.h"
#include "doomgeneric.h"
#include "gpio_input.h"
#include "lcd_init.h"
#include "sdram.h"
#include "sd_driver.h"

#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"

void TM4C_SetSystemClockHz(uint32_t system_clock_hz);
uint32_t ui32Val, ui32Freq, g_ui32SysClock;
extern uint32_t __bss_sdram_start__;
extern uint32_t __bss_sdram_end__;

static void TM4C_ZeroExternalBss(void)
{
    uint32_t *ptr;

    for (ptr = &__bss_sdram_start__; ptr < &__bss_sdram_end__; ++ptr)
    {
        *ptr = 0;
    }
}

void InitGraphics(void) {
    // Set graphics library text rendering defaults.
    // GrRaster16BppDriverInit(g_pui32DisplayBuffer);
    // GrContextInit(&g_sContext, &g_sGrRaster16BppDriver);
}

static void TM4C_StopWithDiagnostic(uint16_t color)
{
    LCD_Clear(color);
    IntMasterDisable();
    while (1)
    {
    }
}

static void TM4C_DrawBits(uint8_t value, uint32_t y_start, uint16_t one_color)
{
    volatile uint16_t *framebuffer = LCD_GetFramebuffer();

    for (uint32_t bit = 0U; bit < 8U; ++bit)
    {
        uint16_t color = (value & (0x80U >> bit)) != 0U ? one_color : 0x0000U;
        uint32_t x_start = 40U + (bit * 50U);

        for (uint32_t y = y_start; y < (y_start + 60U); ++y)
        {
            for (uint32_t x = x_start; x < (x_start + 40U); ++x)
            {
                framebuffer[(y * TM4C_LCD_WIDTH) + x] = color;
            }
        }
    }
}

static void TM4C_StopWithWadDiagnostic(uint8_t stage, uint8_t fresult)
{
    LCD_Clear(0x4208U);
    TM4C_DrawBits(stage, 40U, 0xFFE0U);
    TM4C_DrawBits(fresult, 180U, 0xFFFFU);

    IntMasterDisable();
    while (1)
    {
    }
}

static void TM4C_StopWithCmd0Diagnostic(uint8_t response, bool wait_ready_timed_out)
{
    volatile uint16_t *framebuffer = LCD_GetFramebuffer();

    LCD_Clear(0x8000U);
    for (uint32_t bit = 0U; bit < 8U; ++bit)
    {
        uint16_t color = (response & (0x80U >> bit)) != 0U ? 0xFFFFU : 0x0000U;
        uint32_t x_start = 40U + (bit * 50U);

        for (uint32_t y = 110U; y < 210U; ++y)
        {
            for (uint32_t x = x_start; x < (x_start + 40U); ++x)
            {
                framebuffer[(y * TM4C_LCD_WIDTH) + x] = color;
            }
        }
    }

    for (uint32_t y = 240U; y < 260U; ++y)
    {
        uint16_t color = wait_ready_timed_out ? 0x001FU : 0x07E0U;
        for (uint32_t x = 40U; x < 440U; ++x)
        {
            framebuffer[(y * TM4C_LCD_WIDTH) + x] = color;
        }
    }

    IntMasterDisable();
    while (1)
    {
    }
}

void controlboard_init(void) {
    // The FPU should be enabled because some compilers will use floating-
    // point registers, even for non-floating-point code.  If the FPU is not
    // enabled this will cause a fault.  This also ensures that floating-
    // point operations could be added to this application and would work
    // correctly and use the hardware floating-point unit.  Finally, lazy
    // stacking is enabled for interrupt handlers.  This allows floating-
    // point instructions to be used within interrupt handlers, but at the
    // expense of extra stack usage.
    FPUEnable();
    FPULazyStackingEnable();

    //
    // Set the clocking to run at 120MHz from the PLL.
    //
    SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);
    g_ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);		//SYSCTL_CFG_VCO_480


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    // 	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    // 	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);

    // 	//Unlock and commit NMI pins PD7 and PE7 /*PF0*/
    // // 	HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = 0x4C4F434B;
    // // 	HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= 0x1;
    //


    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK)) { }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) { }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOP)) { }
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4 | GPIO_PIN_6);	// ethernet LED0, LED1
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);					// ethernet LED2
    // MotorVoltageSwitchInit();

    SysCtlPeripheralEnable(SYSCTL_PERIPH_LCD0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_LCD0)) { }
    if (LCDRasterEnabled(LCD0_BASE)) {
        LCDRasterDisable(LCD0_BASE);
    }


    // SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    // while (!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0)) { }
    // if(EEPROMInit() == EEPROM_INIT_ERROR) {
        // EEPROMMassErase();
    // }


    // AnalogComparatorInit();
    // KeybInit();
    InitSDRAM();
    // 	TestSDRAM();

    InitLCDModule(g_ui32SysClock);
    InitST7796S_RGB(g_ui32SysClock);
    InitGraphics();


    // UserInit();

}


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


    // system_clock_hz = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
    //                                       SYSCTL_OSC_MAIN |
    //                                       SYSCTL_USE_PLL |
    //                                       SYSCTL_CFG_VCO_480),
    //                                      TM4C_SYSTEM_CLOCK_HZ);

    controlboard_init();
    TM4C_SetSystemClockHz(g_ui32SysClock);
    TM4C_ZeroExternalBss();

    LCD_Init();
    GPIO_InputInit();

    LCD_Clear(0x001FU);
    if (!SD_DriverBusInit())
    {
        TM4C_StopWithDiagnostic(0xF800U);
    }

    LCD_Clear(0xFFE0U);
    switch (SD_DriverCardInit())
    {
        case SD_CARD_INIT_OK:
            break;
        case SD_CARD_INIT_BUS_ERROR:
            TM4C_StopWithDiagnostic(0xF800U);
            break;
        case SD_CARD_INIT_CMD0_ERROR:
            TM4C_StopWithCmd0Diagnostic(SD_DriverCmd0Response(),
                                        SD_DriverCmd0WaitReadyTimedOut());
            break;
        case SD_CARD_INIT_CMD8_ERROR:
            TM4C_StopWithDiagnostic(0xF81FU);
            break;
        case SD_CARD_INIT_ACMD41_ERROR:
            TM4C_StopWithDiagnostic(0xFD20U);
            break;
        case SD_CARD_INIT_CMD1_ERROR:
            TM4C_StopWithDiagnostic(0xA145U);
            break;
        case SD_CARD_INIT_CMD58_ERROR:
            TM4C_StopWithDiagnostic(0x780FU);
            break;
        case SD_CARD_INIT_CMD16_ERROR:
            TM4C_StopWithDiagnostic(0xFFE0U);
            break;
        default:
            TM4C_StopWithDiagnostic(0xF800U);
            break;
    }

    LCD_Clear(0x07FFU);
    if (!SD_DriverMount())
    {
        TM4C_StopWithDiagnostic(0xFFFFU);
    }

    LCD_Clear(0x7BEFU);
    {
        sd_wad_check_result_t wad_result = SD_DriverWadCheck(TM4C_WAD_PATH);

        if (wad_result != SD_WAD_CHECK_OK)
        {
            TM4C_StopWithWadDiagnostic((uint8_t)wad_result, SD_DriverLastFatFsResult());
        }
    }

    LCD_Clear(0x07E0U);
    // doomgeneric_Create(6, doom_argv);

    while (1)
    {
        // doomgeneric_Tick();
    }
}
