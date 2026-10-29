#include <stdbool.h>
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

static void TM4C_StopWithAddress(uint32_t address, uint32_t value);

// Test čtení/zápisu BSS oblasti v SDRAM; při chybě zobrazí adresu (bitově).
static void TM4C_SdramBssTest(void)
{
    volatile uint32_t *start = &__bss_sdram_start__;
    volatile uint32_t *end = &__bss_sdram_end__;
    volatile uint32_t *p;

    for (p = start; p < end; ++p)
    {
        if (*p != 0U)
        {
            TM4C_StopWithAddress((uint32_t)p, *p);
        }
    }
    for (p = start; p < end; ++p)
    {
        *p = (uint32_t)p ^ 0xA5A5A5A5UL;
    }
    for (p = start; p < end; ++p)
    {
        if (*p != ((uint32_t)p ^ 0xA5A5A5A5UL))
        {
            TM4C_StopWithAddress((uint32_t)p, *p);
        }
    }
    for (p = start; p < end; ++p)
    {
        *p = 0U;
    }
}

// Zátěžové čtení SDRAM vzorem "načti ukazatel z literálu, pak hodnotu"; hodnota má být 0.
static void TM4C_SdramReadStress(void)
{
    static volatile uint32_t *const probe = (volatile uint32_t *)0x1005085CUL;
    static volatile uint32_t *const other = (volatile uint32_t *)0x1005065CUL;

    for (uint32_t i = 0U; i < 2000000UL; ++i)
    {
        uint32_t a = *probe;
        uint32_t b = *other;
        if ((a != 0U) || (b != 0U))
        {
            TM4C_StopWithAddress(a, b);
        }
    }
}

// Test aliasingu SDRAM: po 1 MiB zapíše unikátní hodnotu a ověří, že se nic nezrcadlí.
static void TM4C_SdramAliasTest(void)
{
    const uint32_t step = 0x00100000UL;
    const uint32_t off = 0x00080000UL;
    uint32_t k;

    for (k = 0U; k < 32U; ++k)
    {
        *(volatile uint32_t *)(TM4C_SDRAM_BASE + (k * step) + off) = 0xC0DE0000UL + k;
    }
    for (k = 0U; k < 32U; ++k)
    {
        uint32_t addr = TM4C_SDRAM_BASE + (k * step) + off;
        uint32_t v = *(volatile uint32_t *)addr;

        if (v != (0xC0DE0000UL + k))
        {
            TM4C_StopWithAddress(addr, v);
        }
    }
    for (k = 0U; k < 32U; ++k)
    {
        *(volatile uint32_t *)(TM4C_SDRAM_BASE + (k * step) + off) = 0U;
    }
}

// Test retence a plného rozsahu: zapíše vzor do zone, počká 3 s a ověří; pak totéž s inverzí.
// Při chybě: horní řádek = adresa, dolní = XOR(přečteno, očekáváno).
static void TM4C_SdramRetentionTest(void)
{
    volatile uint32_t *start = (volatile uint32_t *)TM4C_DOOM_ZONE_HEAP_ADDR;
    volatile uint32_t *end = (volatile uint32_t *)(TM4C_DOOM_ZONE_HEAP_ADDR + TM4C_DOOM_ZONE_HEAP_SIZE);
    volatile uint32_t *p;

    for (uint32_t pass = 0U; pass < 2U; ++pass)
    {
        uint32_t mask = (pass == 0U) ? 0x5A5AA5A5UL : 0xA5A55A5AUL;

        for (p = start; p < end; ++p)
        {
            *p = (uint32_t)p ^ mask;
        }

        SysCtlDelay((g_ui32SysClock / 3UL) * 3UL);

        for (p = start; p < end; ++p)
        {
            uint32_t expected = (uint32_t)p ^ mask;
            uint32_t v = *p;

            if (v != expected)
            {
                TM4C_StopWithAddress((uint32_t)p, v ^ expected);
            }
        }
    }

    for (p = start; p < end; ++p)
    {
        *p = 0U;
    }
}

// Náhodný přístup do zone: dvojité čtení téhož slova + bajtové/půlslovní zápisy.
// Při chybě: horní řádek = adresa, dolní = hodnota (1. čtení), prostřední = 2. čtení.
static void TM4C_SdramRandomTest(void)
{
    const uint32_t base = TM4C_DOOM_ZONE_HEAP_ADDR;
    const uint32_t words = TM4C_DOOM_ZONE_HEAP_SIZE / 4U;
    volatile uint32_t *mem = (volatile uint32_t *)base;
    uint32_t lfsr = 0xACE1U;
    uint32_t i;

    for (i = 0U; i < words; ++i)
    {
        mem[i] = (uint32_t)&mem[i] ^ 0x3C3C5A5AUL;
    }

    for (i = 0U; i < 2000000UL; ++i)
    {
        uint32_t idx;
        uint32_t expected;
        uint32_t a;
        uint32_t b;

        lfsr = (lfsr >> 1) ^ (uint32_t)(-(int32_t)(lfsr & 1U) & 0xD0000001UL);
        idx = lfsr % words;
        expected = (uint32_t)&mem[idx] ^ 0x3C3C5A5AUL;

        a = mem[idx];
        b = mem[idx];
        if ((a != expected) || (b != expected))
        {
            TM4C_DrawWord(b, 200U, 0x07FFU);
            TM4C_StopWithAddress((uint32_t)&mem[idx], a ^ expected);
        }

        if ((i & 3U) == 0U)
        {
            volatile uint8_t *bp = (volatile uint8_t *)&mem[idx];

            bp[i & 3U] = bp[i & 3U];
            bp[1] = bp[1];
            if (mem[idx] != expected)
            {
                TM4C_StopWithAddress((uint32_t)&mem[idx], mem[idx] ^ expected);
            }
            ((volatile uint16_t *)&mem[idx])[1] = (uint16_t)(expected >> 16);
            if (mem[idx] != expected)
            {
                TM4C_StopWithAddress((uint32_t)&mem[idx], mem[idx] ^ expected);
            }
        }
    }

    for (i = 0U; i < words; ++i)
    {
        mem[i] = 0U;
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
        uint32_t x_start = 16U + (bit * 38U);

        for (uint32_t y = y_start; y < (y_start + 60U); ++y)
        {
            for (uint32_t x = x_start; x < (x_start + 30U); ++x)
            {
                framebuffer[(y * TM4C_LCD_WIDTH) + x] = color;
            }
        }
    }
}

static void TM4C_DrawWord(uint32_t value, uint32_t y_start, uint16_t color)
{
    volatile uint16_t *fb = LCD_GetFramebuffer();

    for (uint32_t bit = 0U; bit < 32U; ++bit)
    {
        uint16_t c = (value & (0x80000000UL >> bit)) != 0U ? color : 0x0000U;

        for (uint32_t y = y_start; y < (y_start + 30U); ++y)
        {
            for (uint32_t x = bit * 10U; x < (bit * 10U) + 8U; ++x)
            {
                fb[(y * TM4C_LCD_WIDTH) + x] = c;
            }
        }
    }
}

// Fialová/oranžová pozadí: horní řádek = adresa, dolní = přečtená hodnota.
static void TM4C_StopWithAddress(uint32_t address, uint32_t value)
{
    LCD_Clear(0x780FU);
    TM4C_DrawWord(address, 40U, 0xFFFFU);
    TM4C_DrawWord(value, 120U, 0xFFE0U);

    while (1)
    {
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
        uint32_t x_start = 16U + (bit * 38U);

        for (uint32_t y = 110U; y < 210U; ++y)
        {
            for (uint32_t x = x_start; x < (x_start + 30U); ++x)
            {
                framebuffer[(y * TM4C_LCD_WIDTH) + x] = color;
            }
        }
    }

    for (uint32_t y = 240U; y < 260U; ++y)
    {
        uint16_t color = wait_ready_timed_out ? 0x001FU : 0x07E0U;
        for (uint32_t x = 16U; x < 304U; ++x)
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

    // Debug: vypnout write buffer, aby bus fault byl přesný (PC = chybný zápis).
    *(volatile uint32_t *)0xE000E008UL |= 0x2UL;

    controlboard_init();
    TM4C_SetSystemClockHz(g_ui32SysClock);
    TM4C_ZeroExternalBss();

    LCD_Init();
    // GPIO_InputInit();  // tlačítka dočasně vypnutá (ladění NMI)

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

    TM4C_SdramBssTest();
    TM4C_SdramReadStress();
    TM4C_SdramAliasTest();
    TM4C_SdramRetentionTest();
    TM4C_SdramRandomTest();

    LCD_Clear(0x07E0U);
    doomgeneric_Create(6, doom_argv);

    while (1)
    {
        doomgeneric_Tick();
    }
}
