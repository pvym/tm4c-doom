#include <stdbool.h>

#include "config.h"
#include "lcd_init.h"
#include "st7796s_commands.h"

#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "inc/hw_types.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/lcd.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "pins.h"
#include "Powertip320x480x16_st7796s_spi.h"

#define WriteDataSPI(data) SSIDataPut(LCD_SSI_BASE, (data | 0x100));
#define WriteCommandSPI(command) SSIDataPut(LCD_SSI_BASE, (command));

uint32_t *g_pui32DisplayBuffer = (uint32_t *)LCD_FRAME_BUFFER_ADDR;

void InitPowerTip(uint32_t ui32SysClk) {
    // 	Powertip320x480x16_ToRGB();
}

const tRasterDisplayInfo g_sPowertip480x320x60Hz =
{
    "480x320 at 50Hz on Powertip320x480x16_st7796s",
    LCD_PIXEL_CLOCK,
    SYSCTL_CFG_VCO_480,
    120000000,	// SysCLK
    {
        (RASTER_TIMING_ACTIVE_LOW_PIXCLK |	// toto musi byt - high dela duchy
        RASTER_TIMING_SYNCS_ON_RISING_PIXCLK |
        RASTER_TIMING_ACTIVE_LOW_HSYNC |
        RASTER_TIMING_ACTIVE_LOW_VSYNC |
        RASTER_TIMING_ACTIVE_HIGH_OE),
        LCD_HORIZONTAL_MAX, LCD_VERTICAL_MAX,
        LCD_HFP, //38,//10, //38, // hfp			>=10
        LCD_HBP, //24,//3,  //4, //,  // hbp  4		>=3
        LCD_HPW, //18,//3,	//2,  // hpw / hsw  2  	>=3
        LCD_VFP, //8,//3,  // vfp
        LCD_VBP, //6,//3,  // vbp
        LCD_VS,  //6,//3,  // vs /vsw
        LCD_BIAS_LC //0   //
    },
    InitPowerTip
};

const tRasterDisplayInfo *g_psDisplayMode = &g_sPowertip480x320x60Hz;
#define SCREEN_BPP    16

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


//*****************************************************************************
//
// Video interface timings for an Innolux EJ090NA-03A display with 800x480
// resolution, refreshed at 60Hz.
//
//*****************************************************************************
// const tRasterDisplayInfo g_sPowertip480x320x60Hz =
// {
//     "480x320 at 50Hz on Powertip320x480x16_st7796s",
//     LCD_PIXEL_CLOCK,
//     SYSCTL_CFG_VCO_480,
//     120000000,	// SysCLK
//     {
//         (RASTER_TIMING_ACTIVE_LOW_PIXCLK |	// toto musi byt - high dela duchy
//         RASTER_TIMING_SYNCS_ON_RISING_PIXCLK |
//         RASTER_TIMING_ACTIVE_LOW_HSYNC |
//         RASTER_TIMING_ACTIVE_LOW_VSYNC |
//         RASTER_TIMING_ACTIVE_HIGH_OE),
//         LCD_HORIZONTAL_MAX, LCD_VERTICAL_MAX,
//         LCD_HFP, //38,//10, //38, // hfp			>=10
//         LCD_HBP, //24,//3,  //4, //,  // hbp  4		>=3
//         LCD_HPW, //18,//3,	//2,  // hpw / hsw  2  	>=3
//         LCD_VFP, //8,//3,  // vfp
//         LCD_VBP, //6,//3,  // vbp
//         LCD_VS,  //6,//3,  // vs /vsw
//         LCD_BIAS_LC //0   //
//     },
//     InitPowerTip
// };



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


//*****************************************************************************
//
// Switches Backlight OFF for the LCD Panel
//
//*****************************************************************************

void LEDBacklightOFF(void) {
    GPIOPinWrite(LCD_BACKLIGHT_BASE, LCD_BACKLIGHT_PIN, 0);
}

void LEDBacklightON(void) {
    GPIOPinWrite(LCD_BACKLIGHT_BASE, LCD_BACKLIGHT_PIN, LCD_BACKLIGHT_PIN);
}


void InitLCDGpioForRGBSPI(void) {
    // GPIO nepouzite vystupy na 0
    // 	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOR);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOS);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOT);
    // 	while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) { }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) { }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOH)) { }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)) { }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPION)) { }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOR)) { }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOS)) { }
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOT)) { }

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LCD_PIN_DRDX | LCD_PIN_DRESET);
    GPIOPinTypeGPIOOutput(LCD_PIN_BACKLIGHT_PORT, LCD_PIN_BACKLIGHT);

    // Vsechny piny LCD do 0
    // 	GPIOPinWrite(GPIO_PORTF_BASE, LCD_PIN_DRDX | LCD_PIN_DRESET | LCD_PIN_DD2, 0);			// RESET active
    GPIOPinWrite(GPIO_PORTF_BASE, LCD_PIN_DRDX | LCD_PIN_DRESET, LCD_PIN_DRDX | LCD_PIN_DRESET);			// RESET active, DRDX=1
    GPIOPinWrite(LCD_PIN_BACKLIGHT_PORT, LCD_PIN_BACKLIGHT, 0);									// Backlight off
}

//*****************************************************************************
//
// Initializes the pins required for the GPIO-based LCD interface.
//
// This function configures the GPIO pins used to control the LCD display
// when the basic GPIO interface is in use.  On exit, the LCD controller
// has been reset and is ready to receive command and data writes.
//
// \return None.
//
//*****************************************************************************
static void InitSPILCDInterface(uint32_t ui32SysClock) {
    uint32_t pui32DataRx[3];

    // The SSI0 peripheral must be enabled for use.
    SysCtlPeripheralEnable(LCD_SSI_GPIO_PERIPH);
    SysCtlPeripheralEnable(LCD_SSI_PERIPH);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0)) {}

    GPIOPinConfigure(LCD_SSI_CLK_CFG);
    GPIOPinConfigure(LCD_SSI_TX_CFG);
    GPIOPinConfigure(LCD_SSI_RX_CFG);
    GPIOPinConfigure(LCD_SSI_FSS_CFG);

    // Configure the GPIO settings for the SSI pins.  This function also gives
    // TX, RX, FSS, CLK
    GPIOPinTypeSSI(LCD_SSI_GPIO_BASE, LCD_PIN_DCLK | LCD_PIN_DSDA | LCD_PIN_DCSX | LCD_PIN_DSDO);

    // Configure and enable the SSI port for SPI master mode.  Use SSI0
    // 	SSIConfigSetExpClk(LCD_SSI_BASE, ui32SysClock, SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 10000000, 9);
    SSIConfigSetExpClk(LCD_SSI_BASE, ui32SysClock, SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000000, 9);
    // Enable the SSI0 module.
    SSIEnable(LCD_SSI_BASE);

    // Read any residual data from the SSI port.  This makes sure the receive
    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
    // because the SPI SSI mode is full-duplex, which allows you to send and
    // receive at the same time.  The SSIDataGetNonBlocking function returns
    // "true" when data was returned, and "false" when no data was returned.
    // The "non-blocking" function checks if there is any data in the receive
    // FIFO and does not "hang" if there isn't.
    while (SSIDataGetNonBlocking(LCD_SSI_BASE, &pui32DataRx[0])) {}
}


void InitST7796S_RGB(uint32_t ui32SysClock) {
    uint32_t ui32ClockMS;
    // 	uint32_t dataread[8];
    // 	uint32_t *p_dr;

    InitLCDGpioForRGBSPI();		// nastaveni zatim nepouzitych vystupu / vstupu
    // Initializes the SPI Controller for the LCD controller
    InitSPILCDInterface(ui32SysClock);

    // Divide by 3 to get the number of SysCtlDelay loops in 1mS.
    // 	ui32ClockMS = ui32SysClock / (3 * 1000);
    ui32ClockMS = ui32SysClock / (3 * 1000);


    // Switch off the LED backlight
    LEDBacklightON();
    SysCtlDelay(10 * ui32ClockMS);

    // Reset the LCD
    GPIOPinWrite(LCD_RST_BASE, LCD_RST_PIN, 0);
    SysCtlDelay(400 * ui32ClockMS);


    // Reset RELEASE
    GPIOPinWrite(LCD_RST_BASE, LCD_RST_PIN, LCD_RST_PIN);
    SysCtlDelay(400 * ui32ClockMS);

    WriteCommandSPI(ST7796S_SWRESET);
    SysCtlDelay(120 * ui32ClockMS);

    // enable commands 2
    WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
    WriteDataSPI(0xc3);					// D[7:0] = C3h enable command 2 part I
    WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
    WriteDataSPI(0x96);					// D[7:0] = C3h enable command 2 part II

    WriteCommandSPI(ST7796S_SLPOUT);
    SysCtlDelay(120 * ui32ClockMS);

    WriteCommandSPI(ST7796S_IDMOFF);


    WriteCommandSPI(ST7796S_MADCTL);	// Memory Data Access Control
    WriteDataSPI(0x08);					// 0x48 MV   - m.j. i RGB/BGR data order

    WriteCommandSPI(ST7796S_COLMOD);
    WriteDataSPI(0x55);

    WriteCommandSPI(ST7796S_DIC);		// Display Inversion Control
    WriteDataSPI(0x01);

    WriteCommandSPI(ST7796S_IFMODE);	// Display Function Control
    WriteDataSPI(0x00);


    //FIXME zkusit zmenit nastaveni
    // minimalne prvni parametr 30 by asi nemel 30 ale 60 - t.j. 4.bit by mel byt v nule
    WriteCommandSPI(ST7796S_DFC);		// Display Function Control
    WriteDataSPI(0x20);		// 0x20 RGB interface  + DE_MODE - LCDAC - dela DE signal
    WriteDataSPI(0x22);		//0x22	// SS=1  ISC=02
    WriteDataSPI(0x3B);		// 3B- 59 - 60*8 480

    // 	// TODO 2022-01-26 - zkontrolovat dle datasheetu - 3. param by mel byt nula a ostatni zkontrolovat
    WriteCommandSPI(ST7796S_BPC);		// Display Function Control
    WriteDataSPI(g_sPowertip480x320x60Hz.sTiming.ui8VFrontPorch);
    WriteDataSPI(g_sPowertip480x320x60Hz.sTiming.ui8VBackPorch);
    // 	WriteDataSPI(((g_sPowertip480x320x60Hz.sTiming.ui16HFrontPorch) & 0xFF));		// ILI9488 toto ma
    WriteDataSPI(0x00); //- u ST7796S je 3. parametr 0
    WriteDataSPI(((g_sPowertip480x320x60Hz.sTiming.ui16HBackPorch) & 0xFF));

    WriteCommandSPI(ST7796S_WRCABC);
    WriteDataSPI(0x82);

    // disable commands 2
    WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
    WriteDataSPI(0x3c);					// 3c disable command 2 part I
    WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
    WriteDataSPI(0x69);					// 69 disable  command 2 part II

    SysCtlDelay(80 * ui32ClockMS);

    WriteCommandSPI(ST7796S_DISPON);
    SysCtlDelay(60 * ui32ClockMS);

    // Switch on the LED backlight
    LEDBacklightON();

    // 	p_dr = dataread;
    // 	WriteCommandSPI(ST7796S_RDDSM);		// Command Set Control
    // 	SSIDataGet(LCD_SSI_BASE, p_dr++);
    // 	SSIDataGet(LCD_SSI_BASE, p_dr++);
    // 	SSIDataGet(LCD_SSI_BASE, p_dr++);
    // 	SSIDataGet(LCD_SSI_BASE, p_dr++);
    // 	SSIDataGet(LCD_SSI_BASE, p_dr++);
}
#define SIZE_BUFFER ((RASTER_WIDTH * RASTER_HEIGHT * SCREEN_BPP) / 8)
#define SIZE_PALETTE 0 //((SCREEN_BPP == 8) ? (256 * 2) : (16 * 2))


void InitLCDModule(uint32_t ui32SysClkHz) {
    // TODO upravit HW konfiguraci

    //
    // Enable the GPIO peripherals used to interface to the LCD panel.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_LCD0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_LCD0)) { };

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOR);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOS);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOT);

    //použití
    // Configure all the LCD controller pins for hardware control.
    //
    GPIOPinTypeLCD(GPIO_PORTF_BASE, 0x80);		//1
    GPIOPinTypeLCD(GPIO_PORTJ_BASE, 0x4C);		//3
    GPIOPinTypeLCD(GPIO_PORTN_BASE, 0xC0);		//2
    GPIOPinTypeLCD(GPIO_PORTR_BASE, 0xFF);		//8
    GPIOPinTypeLCD(GPIO_PORTS_BASE, 0xF0);		//4
    GPIOPinTypeLCD(GPIO_PORTT_BASE, 0x03);		//2

    //
    // Set the pin muxing to ensure that LCD signals appear on the required
    // pins.  Note that we take advantage of the fact that we know the mux
    // selector is F here to allow us to OR the value without first masking
    // off anything that may have been there before.
    //
    HWREG(GPIO_PORTF_BASE + GPIO_O_PCTL) |= 0xF0000000;
    HWREG(GPIO_PORTJ_BASE + GPIO_O_PCTL) |= 0x0F00FF00;
    HWREG(GPIO_PORTN_BASE + GPIO_O_PCTL) |= 0xFF000000;
    HWREG(GPIO_PORTR_BASE + GPIO_O_PCTL) |= 0xFFFFFFFF;
    HWREG(GPIO_PORTS_BASE + GPIO_O_PCTL) |= 0xFFFF0000;
    HWREG(GPIO_PORTT_BASE + GPIO_O_PCTL) |= 0x000000FF;

    // 	LCDRasterDisable(LCD0_BASE);

    //
    // Set the output format for the raster interface.
    //
    LCDRasterConfigSet(LCD0_BASE, (RASTER_FMT_ACTIVE_PALETTIZED_16BIT | RASTER_LOAD_DATA_ONLY ), 0);		//


    //
    // Configure the LCD controller for raster operation with a pixel clock
    // as close to the requested pixel clock as possible.
    //
    LCDModeSet(LCD0_BASE, (LCD_MODE_RASTER | LCD_MODE_AUTO_UFLOW_RESTART), g_psDisplayMode->ui32PixClock, ui32SysClkHz);

    LCDRasterTimingSet(LCD0_BASE, &(g_psDisplayMode->sTiming));

    //
    // Configure DMA-related parameters.
    //
    LCDDMAConfigSet(LCD0_BASE, LCD_DMA_BURST_16 | LCD_DMA_FIFORDY_64_WORDS | LCD_DMA_BYTE_ORDER_0123);

    //
    // If the chosen display has an initialization function, call it now.
    //
    // 	if(g_psDisplayMode->pfnInitDisplay)
    // 	{
    // 		g_psDisplayMode->pfnInitDisplay(ui32SysClkHz);
    // 	}

    //
    // Set up the frame buffer.  Note that we allow this buffer to extend
    // outside the available SRAM.  This allows us to easily test modes where
    // we can't fit the whole frame in memory, realizing, of course, that
    // part of the display will contain crud.
    //
    LCDRasterFrameBufferSet(LCD0_BASE, 0, g_pui32DisplayBuffer, SIZE_BUFFER);

    // 	for(int ui32Loop = SIZE_PALETTE; ui32Loop < ((SIZE_BUFFER+SIZE_PALETTE) / sizeof(uint32_t)); ui32Loop++)
    // 	{
    // 		g_pui32DisplayBuffer[ui32Loop] = 0;
    // 	}

    //
    // Enable the LCD interrupts.
    //
    LCDIntRegister(LCD0_BASE, LCDIntHandler);
    LCDIntEnable(LCD0_BASE, (LCD_INT_DMA_DONE |
    LCD_INT_SYNC_LOST |
    LCD_INT_UNDERFLOW |
    LCD_INT_AC_BIAS_CNT |
    LCD_INT_RASTER_FRAME_DONE |
    LCD_INT_PAL_LOAD |
    LCD_INT_EOF1 |
    LCD_INT_EOF0));

    IntEnable(INT_LCD0);

    //
    // Enable the raster output.
    //
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
