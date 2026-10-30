#include "tm4c_lcd_st7796s.h"

#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/lcd.h"
#include "driverlib/sysctl.h"

/* Příkazy ST7796S. */
#define ST7796S_SWRESET 0x01U
#define ST7796S_SLPOUT  0x11U
#define ST7796S_DISPON  0x29U
#define ST7796S_MADCTL  0x36U
#define ST7796S_IDMOFF  0x38U
#define ST7796S_COLMOD  0x3AU
#define ST7796S_WRCABC  0x55U
#define ST7796S_IFMODE  0xB0U
#define ST7796S_DIC     0xB4U
#define ST7796S_BPC     0xB5U
#define ST7796S_DFC     0xB6U
#define ST7796S_CSCON   0xF0U

#define PERIPH_TIMEOUT 1000000U
#define SWAP_GUARD     3000000U

/* Piny LCD0 (RGB565 + řízení); funkce 15 v PCTL. */
typedef struct
{
    uint32_t periph;
    uint32_t base;
    uint8_t pins;
} lcd_port_t;

static const lcd_port_t s_lcd_ports[] =
{
    { SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, 0x80U },
    { SYSCTL_PERIPH_GPIOJ, GPIO_PORTJ_BASE, 0x4CU },
    { SYSCTL_PERIPH_GPION, GPIO_PORTN_BASE, 0xC0U },
    { SYSCTL_PERIPH_GPIOR, GPIO_PORTR_BASE, 0xFFU },
    { SYSCTL_PERIPH_GPIOS, GPIO_PORTS_BASE, 0xF0U },
    { SYSCTL_PERIPH_GPIOT, GPIO_PORTT_BASE, 0x03U },
};

#define LCD_PORT_COUNT (sizeof(s_lcd_ports) / sizeof(s_lcd_ports[0]))

static const tm4c_lcd_cfg_t *s_cfg;
static volatile uintptr_t s_front_addr;
static volatile uintptr_t s_pending_addr;
static volatile uint32_t s_underflow_count;
static volatile uint32_t s_synclost_count;
static volatile uint32_t s_frame_count;

static bool WaitReady(uint32_t periph)
{
    for (uint32_t timeout = PERIPH_TIMEOUT; timeout > 0U; --timeout)
    {
        if (SysCtlPeripheralReady(periph))
        {
            return true;
        }
    }

    return false;
}

static uint32_t FramebufferSize(void)
{
    return (uint32_t)s_cfg->width * s_cfg->height * sizeof(uint16_t);
}

static bool ControllerGpioInit(void)
{
    for (uint32_t i = 0U; i < LCD_PORT_COUNT; ++i)
    {
        SysCtlPeripheralEnable(s_lcd_ports[i].periph);
    }

    for (uint32_t i = 0U; i < LCD_PORT_COUNT; ++i)
    {
        const lcd_port_t *port = &s_lcd_ports[i];
        uint32_t pctl;

        if (!WaitReady(port->periph))
        {
            return false;
        }

        GPIOPinTypeLCD(port->base, port->pins);

        pctl = HWREG(port->base + GPIO_O_PCTL);
        for (uint32_t bit = 0U; bit < 8U; ++bit)
        {
            if ((port->pins & (1U << bit)) != 0U)
            {
                pctl |= 0xFUL << (4U * bit);
            }
        }
        HWREG(port->base + GPIO_O_PCTL) = pctl;
    }

    return true;
}

static void ControllerStart(void)
{
    tLCDRasterTiming timing;

    timing.ui32Flags = RASTER_TIMING_ACTIVE_LOW_PIXCLK |
                       RASTER_TIMING_SYNCS_ON_RISING_PIXCLK |
                       RASTER_TIMING_ACTIVE_LOW_HSYNC |
                       RASTER_TIMING_ACTIVE_LOW_VSYNC |
                       RASTER_TIMING_ACTIVE_HIGH_OE;
    timing.ui16PanelWidth = s_cfg->width;
    timing.ui16PanelHeight = s_cfg->height;
    timing.ui16HFrontPorch = s_cfg->h_front_porch;
    timing.ui16HBackPorch = s_cfg->h_back_porch;
    timing.ui16HSyncWidth = s_cfg->h_pulse_width;
    timing.ui8VFrontPorch = s_cfg->v_front_porch;
    timing.ui8VBackPorch = s_cfg->v_back_porch;
    timing.ui8VSyncWidth = s_cfg->v_pulse_width;
    timing.ui8ACBiasLineCount = 0U;

    s_front_addr = s_cfg->framebuffer[0];
    s_pending_addr = 0U;

    LCDRasterConfigSet(LCD0_BASE,
                       RASTER_FMT_ACTIVE_PALETTIZED_16BIT | RASTER_LOAD_DATA_ONLY,
                       0U);
    LCDModeSet(LCD0_BASE,
               LCD_MODE_RASTER | LCD_MODE_AUTO_UFLOW_RESTART,
               s_cfg->pixel_clock_hz,
               s_cfg->sysclk_hz);
    LCDRasterTimingSet(LCD0_BASE, &timing);
    LCDDMAConfigSet(LCD0_BASE,
                    LCD_DMA_BURST_16 |
                    LCD_DMA_FIFORDY_64_WORDS |
                    LCD_DMA_BYTE_ORDER_0123);
    LCDRasterFrameBufferSet(LCD0_BASE, 0U, (void *)s_front_addr, FramebufferSize());

    LCDIntRegister(LCD0_BASE, Tm4cLcdIntHandler);
    LCDIntEnable(LCD0_BASE,
                 LCD_INT_SYNC_LOST | LCD_INT_UNDERFLOW | LCD_INT_RASTER_FRAME_DONE);
    IntEnable(INT_LCD0);
    LCDRasterEnable(LCD0_BASE);
}

void Tm4cLcdIntHandler(void)
{
    uint32_t status = LCDIntStatus(LCD0_BASE, true);

    LCDIntClear(LCD0_BASE, status);

    /* Adresu měníme až po dokončení snímku, aby nevznikal tearing. */
    if (((status & LCD_INT_RASTER_FRAME_DONE) != 0U) && (s_pending_addr != 0U))
    {
        LCDRasterFrameBufferSet(LCD0_BASE, 0U, (void *)s_pending_addr, FramebufferSize());
        s_front_addr = s_pending_addr;
        s_pending_addr = 0U;
    }

    if ((status & LCD_INT_UNDERFLOW) != 0U)
    {
        ++s_underflow_count;
    }
    if ((status & LCD_INT_SYNC_LOST) != 0U)
    {
        ++s_synclost_count;
    }
    if ((status & (LCD_INT_UNDERFLOW | LCD_INT_SYNC_LOST)) != 0U)
    {
        /* Plný restart rastru a DMA na aktuálním front bufferu. */
        LCDRasterDisable(LCD0_BASE);
        LCDRasterFrameBufferSet(LCD0_BASE, 0U, (void *)s_front_addr, FramebufferSize());
        LCDRasterEnable(LCD0_BASE);
    }
    if ((status & LCD_INT_RASTER_FRAME_DONE) != 0U)
    {
        ++s_frame_count;
    }
}

static void DelayMs(uint32_t ms)
{
    SysCtlDelay((s_cfg->sysclk_hz / 3000U) * ms);
}

static void PanelCommand(uint8_t command)
{
    (void)Tm4cSpiTransfer(&s_cfg->panel_spi, command);
}

static void PanelData(uint8_t data)
{
    (void)Tm4cSpiTransfer(&s_cfg->panel_spi, 0x100U | data);
}

static bool PanelGpioInit(void)
{
    const tm4c_gpio_t *outputs[3] = { &s_cfg->reset, &s_cfg->backlight, &s_cfg->drdx };

    for (uint32_t i = 0U; i < 3U; ++i)
    {
        if (outputs[i]->pin != 0U)
        {
            SysCtlPeripheralEnable(outputs[i]->gpio_periph);
        }
    }
    for (uint32_t i = 0U; i < 3U; ++i)
    {
        if (outputs[i]->pin != 0U)
        {
            if (!WaitReady(outputs[i]->gpio_periph))
            {
                return false;
            }
            GPIOPinTypeGPIOOutput(outputs[i]->gpio_base, outputs[i]->pin);
        }
    }

    if (s_cfg->drdx.pin != 0U)
    {
        GPIOPinWrite(s_cfg->drdx.gpio_base, s_cfg->drdx.pin, s_cfg->drdx.pin);
    }
    GPIOPinWrite(s_cfg->reset.gpio_base, s_cfg->reset.pin, s_cfg->reset.pin);
    Tm4cLcdBacklight(false);
    return true;
}

static void PanelInit(void)
{
    DelayMs(10U);
    GPIOPinWrite(s_cfg->reset.gpio_base, s_cfg->reset.pin, 0U);
    DelayMs(400U);
    GPIOPinWrite(s_cfg->reset.gpio_base, s_cfg->reset.pin, s_cfg->reset.pin);
    DelayMs(400U);

    PanelCommand(ST7796S_SWRESET);
    DelayMs(120U);

    /* Povolit rozšířené příkazy (command set 2). */
    PanelCommand(ST7796S_CSCON);
    PanelData(0xC3U);
    PanelCommand(ST7796S_CSCON);
    PanelData(0x96U);

    PanelCommand(ST7796S_SLPOUT);
    DelayMs(120U);

    PanelCommand(ST7796S_IDMOFF);
    PanelCommand(ST7796S_MADCTL);
    PanelData(s_cfg->madctl);
    PanelCommand(ST7796S_COLMOD);
    PanelData(0x55U);                   /* RGB565, 16 bit */
    PanelCommand(ST7796S_DIC);
    PanelData(0x01U);                   /* inverze po řádcích */
    PanelCommand(ST7796S_IFMODE);
    PanelData(0x00U);

    PanelCommand(ST7796S_DFC);
    PanelData(0x20U);                   /* RGB rozhraní + DE mód */
    PanelData(0x22U);
    PanelData(0x3BU);                   /* 480 řádků */

    PanelCommand(ST7796S_BPC);
    PanelData(s_cfg->v_front_porch);
    PanelData(s_cfg->v_back_porch);
    PanelData(0x00U);
    PanelData(s_cfg->h_back_porch);

    PanelCommand(ST7796S_WRCABC);
    PanelData(0x82U);

    /* Zakázat rozšířené příkazy. */
    PanelCommand(ST7796S_CSCON);
    PanelData(0x3CU);
    PanelCommand(ST7796S_CSCON);
    PanelData(0x69U);

    DelayMs(80U);
    PanelCommand(ST7796S_DISPON);
    DelayMs(60U);
}

bool Tm4cLcdInit(const tm4c_lcd_cfg_t *cfg)
{
    s_cfg = cfg;
    s_underflow_count = 0U;
    s_synclost_count = 0U;
    s_frame_count = 0U;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_LCD0);
    if (!WaitReady(SYSCTL_PERIPH_LCD0))
    {
        return false;
    }
    if (LCDRasterEnabled(LCD0_BASE))
    {
        LCDRasterDisable(LCD0_BASE);
    }

    if (!ControllerGpioInit() || !PanelGpioInit())
    {
        return false;
    }

    /* Raster běží už při inicializaci panelu, panel potřebuje pixel clock. */
    ControllerStart();

    if (!Tm4cSpiInit(&cfg->panel_spi, cfg->sysclk_hz, cfg->panel_spi_hz))
    {
        return false;
    }

    PanelInit();
    Tm4cLcdBacklight(true);
    return true;
}

void Tm4cLcdBacklight(bool on)
{
    GPIOPinWrite(s_cfg->backlight.gpio_base, s_cfg->backlight.pin,
                 on ? s_cfg->backlight.pin : 0U);
}

volatile uint16_t *Tm4cLcdFrontBuffer(void)
{
    return (volatile uint16_t *)s_front_addr;
}

volatile uint16_t *Tm4cLcdBackBuffer(void)
{
    uintptr_t back;

    if (s_cfg->framebuffer[1] == 0U)
    {
        return (volatile uint16_t *)s_front_addr;
    }

    back = (s_front_addr == s_cfg->framebuffer[0]) ? s_cfg->framebuffer[1]
                                                    : s_cfg->framebuffer[0];
    return (volatile uint16_t *)back;
}

void Tm4cLcdSwap(void)
{
    uint32_t guard = SWAP_GUARD;
    uintptr_t back = (uintptr_t)Tm4cLcdBackBuffer();

    if (s_cfg->framebuffer[1] == 0U)
    {
        return;
    }

    s_pending_addr = back;
    while ((s_pending_addr != 0U) && (--guard != 0U))
    {
    }

    if (s_pending_addr != 0U)
    {
        /* Přerušení snímku nepřišlo; přepneme přímo. */
        s_pending_addr = 0U;
        LCDRasterFrameBufferSet(LCD0_BASE, 0U, (void *)back, FramebufferSize());
        s_front_addr = back;
    }
}

void Tm4cLcdClear(uint16_t color)
{
    volatile uint16_t *fb = Tm4cLcdBackBuffer();
    const uint32_t pixels = (uint32_t)s_cfg->width * s_cfg->height;

    for (uint32_t i = 0U; i < pixels; ++i)
    {
        fb[i] = color;
    }
}

uint32_t Tm4cLcdUnderflowCount(void)
{
    return s_underflow_count;
}

uint32_t Tm4cLcdSyncLostCount(void)
{
    return s_synclost_count;
}

uint32_t Tm4cLcdFrameCount(void)
{
    return s_frame_count;
}
