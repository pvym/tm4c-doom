#include "config.h"
#include "lcd_init.h"

#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/lcd.h"
#include "driverlib/sysctl.h"

static const tLCDRasterTiming g_tm4cRasterTiming =
{
    TM4C_LCD_RASTER_FLAGS,
    TM4C_LCD_WIDTH,
    TM4C_LCD_HEIGHT,
    TM4C_LCD_H_FRONT_PORCH,
    TM4C_LCD_H_BACK_PORCH,
    TM4C_LCD_H_PULSE_WIDTH,
    TM4C_LCD_V_FRONT_PORCH,
    TM4C_LCD_V_BACK_PORCH,
    TM4C_LCD_V_PULSE_WIDTH,
    TM4C_LCD_AC_BIAS
};

void LCDIntHandler(void)
{
    uint32_t status = LCDIntStatus(LCD0_BASE, true);

    LCDIntClear(LCD0_BASE, status);

    if ((status & LCD_INT_UNDERFLOW) != 0U)
    {
        LCDRasterEnable(LCD0_BASE);
    }
}

volatile uint16_t *LCD_GetFramebuffer(void)
{
    return (volatile uint16_t *)(uintptr_t)TM4C_LCD_FRAMEBUFFER_ADDR;
}

void __attribute__((weak)) TM4C_LCD_ControllerInit(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_LCD0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_LCD0))
    {
    }

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOR);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOS);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOT);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPION) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOR) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOS) ||
           !SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOT))
    {
    }

    GPIOPinTypeLCD(GPIO_PORTF_BASE, 0x80U);
    GPIOPinTypeLCD(GPIO_PORTJ_BASE, 0x4CU);
    GPIOPinTypeLCD(GPIO_PORTN_BASE, 0xC0U);
    GPIOPinTypeLCD(GPIO_PORTR_BASE, 0xFFU);
    GPIOPinTypeLCD(GPIO_PORTS_BASE, 0xF0U);
    GPIOPinTypeLCD(GPIO_PORTT_BASE, 0x03U);

    HWREG(GPIO_PORTF_BASE + GPIO_O_PCTL) |= 0xF0000000U;
    HWREG(GPIO_PORTJ_BASE + GPIO_O_PCTL) |= 0x0F00FF00U;
    HWREG(GPIO_PORTN_BASE + GPIO_O_PCTL) |= 0xFF000000U;
    HWREG(GPIO_PORTR_BASE + GPIO_O_PCTL) |= 0xFFFFFFFFU;
    HWREG(GPIO_PORTS_BASE + GPIO_O_PCTL) |= 0xFFFF0000U;
    HWREG(GPIO_PORTT_BASE + GPIO_O_PCTL) |= 0x000000FFU;

    LCDRasterConfigSet(LCD0_BASE,
                       RASTER_FMT_ACTIVE_PALETTIZED_16BIT | RASTER_LOAD_DATA_ONLY,
                       0U);
    LCDModeSet(LCD0_BASE,
               LCD_MODE_RASTER | LCD_MODE_AUTO_UFLOW_RESTART,
               TM4C_LCD_PIXEL_CLOCK_HZ,
               TM4C_SYSTEM_CLOCK_HZ);
    LCDRasterTimingSet(LCD0_BASE, &g_tm4cRasterTiming);
    LCDDMAConfigSet(LCD0_BASE,
                    LCD_DMA_BURST_16 |
                    LCD_DMA_FIFORDY_64_WORDS |
                    LCD_DMA_BYTE_ORDER_0123);
    LCDRasterFrameBufferSet(LCD0_BASE,
                            0U,
                            (void *)(uintptr_t)TM4C_LCD_FRAMEBUFFER_ADDR,
                            TM4C_LCD_FRAMEBUFFER_SIZE);
    LCDIntRegister(LCD0_BASE, LCDIntHandler);
    LCDIntEnable(LCD0_BASE,
                 LCD_INT_DMA_DONE |
                 LCD_INT_SYNC_LOST |
                 LCD_INT_UNDERFLOW |
                 LCD_INT_AC_BIAS_CNT |
                 LCD_INT_RASTER_FRAME_DONE |
                 LCD_INT_PAL_LOAD |
                 LCD_INT_EOF1 |
                 LCD_INT_EOF0);
    IntEnable(INT_LCD0);
    LCDRasterEnable(LCD0_BASE);
}

void LCD_Clear(uint16_t color)
{
    volatile uint16_t *framebuffer = LCD_GetFramebuffer();

    for (uint32_t i = 0; i < (TM4C_LCD_WIDTH * TM4C_LCD_HEIGHT); ++i)
    {
        framebuffer[i] = color;
    }
}

void LCD_Init(void)
{
    TM4C_LCD_ControllerInit();
    LCD_Clear(0U);
}
