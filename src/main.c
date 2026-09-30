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

void InitGraphics(void) {
    // Set graphics library text rendering defaults.
    // GrRaster16BppDriverInit(g_pui32DisplayBuffer);
    // GrContextInit(&g_sContext, &g_sGrRaster16BppDriver);
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
