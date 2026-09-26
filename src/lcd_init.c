#include <stdbool.h>

#include "config.h"
#include "lcd_init.h"
#include "st7796s_commands.h"

#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/lcd.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"

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

static void PanelBacklightSet(bool enabled)
{
    GPIOPinWrite(TM4C_PANEL_BACKLIGHT_PORT,
                 TM4C_PANEL_BACKLIGHT_PIN,
                 enabled ? TM4C_PANEL_BACKLIGHT_PIN : 0U);
}

static void PanelSPIWrite(uint16_t value)
{
    uint32_t dummy;

    SSIDataPut(TM4C_PANEL_SPI_BASE, value);
    while (SSIBusy(TM4C_PANEL_SPI_BASE))
    {
    }
    SSIDataGet(TM4C_PANEL_SPI_BASE, &dummy);
}

static void PanelWriteCommand(uint8_t command)
{
    PanelSPIWrite(command);
}

static void PanelWriteData(uint8_t data)
{
    PanelSPIWrite(0x100U | data);
}

static void PanelDelayMs(uint32_t system_clock_hz, uint32_t ms)
{
    SysCtlDelay((system_clock_hz / 3000U) * ms);
}

static void PanelSPIInit(uint32_t system_clock_hz)
{
    uint32_t discard;

    SysCtlPeripheralEnable(TM4C_PANEL_SPI_GPIO_PERIPH);
    SysCtlPeripheralEnable(TM4C_PANEL_SPI_PERIPH);
    while (!SysCtlPeripheralReady(TM4C_PANEL_SPI_GPIO_PERIPH) ||
           !SysCtlPeripheralReady(TM4C_PANEL_SPI_PERIPH))
    {
    }

    GPIOPinConfigure(TM4C_PANEL_SPI_CLK_CFG);
    GPIOPinConfigure(TM4C_PANEL_SPI_TX_CFG);
    GPIOPinConfigure(TM4C_PANEL_SPI_RX_CFG);
    GPIOPinConfigure(TM4C_PANEL_SPI_FSS_CFG);
    GPIOPinTypeSSI(TM4C_PANEL_SPI_GPIO_BASE,
                   TM4C_PANEL_SPI_CLK_PIN |
                   TM4C_PANEL_SPI_TX_PIN |
                   TM4C_PANEL_SPI_RX_PIN |
                   TM4C_PANEL_SPI_FSS_PIN);

    SSIDisable(TM4C_PANEL_SPI_BASE);
    SSIConfigSetExpClk(TM4C_PANEL_SPI_BASE,
                       system_clock_hz,
                       SSI_FRF_MOTO_MODE_0,
                       SSI_MODE_MASTER,
                       TM4C_PANEL_SPI_HZ,
                       9U);
    SSIEnable(TM4C_PANEL_SPI_BASE);

    while (SSIDataGetNonBlocking(TM4C_PANEL_SPI_BASE, &discard))
    {
    }
}

static void PanelGPIOInit(void)
{
    SysCtlPeripheralEnable(TM4C_PANEL_RESET_PERIPH);
    SysCtlPeripheralEnable(TM4C_PANEL_DRDX_PERIPH);
    SysCtlPeripheralEnable(TM4C_PANEL_BACKLIGHT_PERIPH);
    while (!SysCtlPeripheralReady(TM4C_PANEL_RESET_PERIPH) ||
           !SysCtlPeripheralReady(TM4C_PANEL_DRDX_PERIPH) ||
           !SysCtlPeripheralReady(TM4C_PANEL_BACKLIGHT_PERIPH))
    {
    }

    GPIOPinTypeGPIOOutput(TM4C_PANEL_RESET_PORT, TM4C_PANEL_RESET_PIN);
    GPIOPinTypeGPIOOutput(TM4C_PANEL_DRDX_PORT, TM4C_PANEL_DRDX_PIN);
    GPIOPinTypeGPIOOutput(TM4C_PANEL_BACKLIGHT_PORT, TM4C_PANEL_BACKLIGHT_PIN);

    GPIOPinWrite(TM4C_PANEL_DRDX_PORT, TM4C_PANEL_DRDX_PIN, TM4C_PANEL_DRDX_PIN);
    GPIOPinWrite(TM4C_PANEL_RESET_PORT, TM4C_PANEL_RESET_PIN, TM4C_PANEL_RESET_PIN);
    PanelBacklightSet(false);
}

static void PanelInitST7796SRGB(uint32_t system_clock_hz)
{
    PanelGPIOInit();
    PanelSPIInit(system_clock_hz);

    PanelDelayMs(system_clock_hz, 10U);
    GPIOPinWrite(TM4C_PANEL_RESET_PORT, TM4C_PANEL_RESET_PIN, 0U);
    PanelDelayMs(system_clock_hz, 400U);
    GPIOPinWrite(TM4C_PANEL_RESET_PORT,
                 TM4C_PANEL_RESET_PIN,
                 TM4C_PANEL_RESET_PIN);
    PanelDelayMs(system_clock_hz, 400U);

    PanelWriteCommand(ST7796S_SWRESET);
    PanelDelayMs(system_clock_hz, 120U);

    PanelWriteCommand(ST7796S_CSCON);
    PanelWriteData(0xC3U);
    PanelWriteCommand(ST7796S_CSCON);
    PanelWriteData(0x96U);

    PanelWriteCommand(ST7796S_SLPOUT);
    PanelDelayMs(system_clock_hz, 120U);

    PanelWriteCommand(ST7796S_IDMOFF);
    PanelWriteCommand(ST7796S_MADCTL);
    PanelWriteData(0x08U);
    PanelWriteCommand(ST7796S_COLMOD);
    PanelWriteData(0x55U);
    PanelWriteCommand(ST7796S_DIC);
    PanelWriteData(0x01U);
    PanelWriteCommand(ST7796S_IFMODE);
    PanelWriteData(0x00U);

    PanelWriteCommand(ST7796S_DFC);
    PanelWriteData(0x20U);
    PanelWriteData(0x22U);
    PanelWriteData(0x3BU);

    PanelWriteCommand(ST7796S_BPC);
    PanelWriteData((uint8_t)g_tm4cRasterTiming.ui8VFrontPorch);
    PanelWriteData((uint8_t)g_tm4cRasterTiming.ui8VBackPorch);
    PanelWriteData(0x00U);
    PanelWriteData((uint8_t)g_tm4cRasterTiming.ui16HBackPorch);

    PanelWriteCommand(ST7796S_WRCABC);
    PanelWriteData(0x82U);

    PanelWriteCommand(ST7796S_CSCON);
    PanelWriteData(0x3CU);
    PanelWriteCommand(ST7796S_CSCON);
    PanelWriteData(0x69U);

    PanelDelayMs(system_clock_hz, 80U);
    PanelWriteCommand(ST7796S_DISPON);
    PanelDelayMs(system_clock_hz, 60U);
    PanelBacklightSet(true);
}

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
    PanelInitST7796SRGB(TM4C_SYSTEM_CLOCK_HZ);

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
