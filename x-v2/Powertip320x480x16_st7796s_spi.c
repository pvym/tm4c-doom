//*****************************************************************************
//
// Powertip320x480x16_ST7796S - inspired by Kentec320x240x16_ssd2119_spi.c driver
// 2018-12-11 Pavel Vymetalek <pavel@vym.cz>
//
// Kentec320x240x16_ssd2119_spi.c - Display driver for the Kentec
//                                  BOOSTXL-K350QVG-S1 TFT display with an
//                                  SSD2119 controller and SPI interface.
//
// Copyright (c) 2012-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************
/*#ifndef PART_TM4C129XNCZAD
#define PART_TM4C129XNCZAD
#endif*/

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
// #include "driverlib/uart.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "grlib/grlib.h"
#include "st7796s_commands.h"
#include "pins.h"
#include "display.h"
#include "Powertip320x480x16_st7796s_spi.h"

// #include "utils/uartstdio.h"

#define WriteDataSPI(data) MAP_SSIDataPut(LCD_SSI_BASE, (data | 0x100));
#define WriteCommandSPI(command) MAP_SSIDataPut(LCD_SSI_BASE, (command));

#define WriteData16SPI(data) MAP_SSIDataPut(LCD_SSI_BASE, ((data >> 8) | 0x100)); \
MAP_SSIDataPut(LCD_SSI_BASE, ((data & 0xFF) | 0x100));


//*****************************************************************************
//
//! \addtogroup kentec320x240x16_ssd2119_spi
//! @{
//
//*****************************************************************************

//*****************************************************************************
//
// This driver operates in four different screen orientations.  They are:
//
// * Portrait - The screen is taller than it is wide, and the flex connector is
//              on the left of the display.  This is selected by defining
//              PORTRAIT.
//
// * Landscape - The screen is wider than it is tall, and the flex connector is
//               on the bottom of the display.  This is selected by defining
//               LANDSCAPE.
//
// * Portrait flip - The screen is taller than it is wide, and the flex
//                   connector is on the right of the display.  This is
//                   selected by defining PORTRAIT_FLIP.
//
// * Landscape flip - The screen is wider than it is tall, and the flex
//                    connector is on the top of the display.  This is
//                    selected by defining LANDSCAPE_FLIP.
//
// These can also be imagined in terms of screen rotation; if portrait mode is
// 0 degrees of screen rotation, landscape is 90 degrees of counter-clockwise
// rotation, portrait flip is 180 degrees of rotation, and landscape flip is
// 270 degress of counter-clockwise rotation.
//
// If no screen orientation is selected, "landscape flip" mode will be used.
//
//*****************************************************************************
/*
#if ! defined(PORTRAIT) && ! defined(PORTRAIT_FLIP) && \
	! defined(LANDSCAPE) && ! defined(LANDSCAPE_FLIP)
	#define LANDSCAPE
#endif
*/
//*****************************************************************************
//
// The dimensions of the LCD panel.
//
//*****************************************************************************

//*****************************************************************************
//
// Various definitions controlling coordinate space mapping and drawing
// direction in the four supported orientations.
//
//*****************************************************************************
	/*
#ifdef PORTRAIT
	#define HORIZ_DIRECTION 0x28
	#define VERT_DIRECTION 0x20
	#define MAPPED_X(x, y) (LCD_HORIZONTAL_MAX-1 - (y))
	#define MAPPED_Y(x, y) (x)
#endif
#ifdef LANDSCAPE
	#define HORIZ_DIRECTION 0x00
	#define VERT_DIRECTION  0x08
	#define MAPPED_X(x, y) (LCD_HORIZONTAL_MAX-1 - (x))
	#define MAPPED_Y(x, y) (LCD_VERTICAL_MAX-1 - (y))
#endif
#ifdef PORTRAIT_FLIP
	#define HORIZ_DIRECTION 0x18
	#define VERT_DIRECTION 0x10
	#define MAPPED_X(x, y) (y)
	#define MAPPED_Y(x, y) (LCD_HORIZONTAL_MAX-1 - (x))
#endif
#ifdef LANDSCAPE_FLIP
	#define HORIZ_DIRECTION 0x30
	#define VERT_DIRECTION  0x38
	#define MAPPED_X(x, y) (x)
	#define MAPPED_Y(x, y) (y)
#endif

	*/


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



void InitLCDGpioForSPI(void) {
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

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LCD_PIN_DRDX | LCD_PIN_DRESET | LCD_PIN_DD2);
	GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, LCD_PIN_BACKLIGHT);
	GPIOPinTypeGPIOOutput(GPIO_PORTJ_BASE, LCD_PIN_DD14 | LCD_PIN_DD15 | LCD_PIN_LCDAC);
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, LCD_PIN_DD13 | LCD_PIN_DD12);
	GPIOPinTypeGPIOOutput(GPIO_PORTR_BASE, LCD_PIN_DOTCLK | LCD_PIN_VSYNC | LCD_PIN_HSYNC | LCD_PIN_DD3 | LCD_PIN_DD0 | LCD_PIN_DD1 | LCD_PIN_DD4 | LCD_PIN_DD5);
	GPIOPinTypeGPIOOutput(GPIO_PORTS_BASE, LCD_PIN_DD6 | LCD_PIN_DD7 | LCD_PIN_DD8 | LCD_PIN_DD9);
	GPIOPinTypeGPIOOutput(GPIO_PORTT_BASE, LCD_PIN_DD10 | LCD_PIN_DD11);


	// Vsechny piny LCD do 0
// 	GPIOPinWrite(GPIO_PORTF_BASE, LCD_PIN_DRDX | LCD_PIN_DRESET | LCD_PIN_DD2, 0);			// RESET active
	GPIOPinWrite(GPIO_PORTF_BASE, LCD_PIN_DRDX | LCD_PIN_DRESET | LCD_PIN_DD2, LCD_PIN_DRDX);			// RESET active, DRDX=1
	GPIOPinWrite(GPIO_PORTH_BASE, LCD_PIN_BACKLIGHT, 0);									// Backlight off
	GPIOPinWrite(GPIO_PORTJ_BASE, LCD_PIN_DD14 | LCD_PIN_DD15 | LCD_PIN_LCDAC, 0);
	GPIOPinWrite(GPIO_PORTN_BASE, LCD_PIN_DD13 | LCD_PIN_DD12, 0);
	GPIOPinWrite(GPIO_PORTR_BASE, LCD_PIN_DOTCLK | LCD_PIN_VSYNC | LCD_PIN_HSYNC | LCD_PIN_DD3 | LCD_PIN_DD0 | LCD_PIN_DD1 | LCD_PIN_DD4 | LCD_PIN_DD5, 0);
	GPIOPinWrite(GPIO_PORTS_BASE, LCD_PIN_DD6 | LCD_PIN_DD7 | LCD_PIN_DD8 | LCD_PIN_DD9, 0);
	GPIOPinWrite(GPIO_PORTT_BASE, LCD_PIN_DD10 | LCD_PIN_DD11, 0);
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

//*****************************************************************************
//
//! Initializes the display driver.
//!
//! \param ui32SysClock is the frequency of the system clock.
//!
//! This function initializes the LCD controller and the SSD2119 display
//! controller on the panel, preparing it to display data.
//!
//! \return None.
//
//*****************************************************************************
void Powertip320x480x16_ST7796SInit(uint32_t ui32SysClock) {
	uint32_t ui32ClockMS;

	InitLCDGpioForSPI();		// nastaveni zatim nepouzitych vystupu / vstupu

	// Divide by 3 to get the number of SysCtlDelay loops in 1mS.
// 	ui32ClockMS = ui32SysClock / (3 * 1000);
	ui32ClockMS = ui32SysClock / (12 * 1000);


	// Switch off the LED backlight
	LEDBacklightON();
	MAP_SysCtlDelay(10 * ui32ClockMS);

	// Reset the LCD
	GPIOPinWrite(LCD_RST_BASE, LCD_RST_PIN, 0);
	MAP_SysCtlDelay(10 * ui32ClockMS);
	MAP_SysCtlDelay(10 * ui32ClockMS);
	MAP_SysCtlDelay(10 * ui32ClockMS);
	MAP_SysCtlDelay(10 * ui32ClockMS);

	// Initializes the SPI Controller for the LCD controller
	InitSPILCDInterface(ui32SysClock);

	// Reset RELEASE
	GPIOPinWrite(LCD_RST_BASE, LCD_RST_PIN, LCD_RST_PIN);
	MAP_SysCtlDelay(20 * ui32ClockMS);
	MAP_SysCtlDelay(20 * ui32ClockMS);
	MAP_SysCtlDelay(20 * ui32ClockMS);
	MAP_SysCtlDelay(20 * ui32ClockMS);

	WriteCommandSPI(ST7796S_SWRESET);
	MAP_SysCtlDelay(120 * ui32ClockMS);

	WriteCommandSPI(ST7796S_SLPOUT);
	MAP_SysCtlDelay(20 * ui32ClockMS);

	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
	WriteDataSPI(0xc3);					// D[7:0] = C3h enable command 2 part I

	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
	WriteDataSPI(0x96);					// D[7:0] = C3h enable command 2 part II

	WriteCommandSPI(ST7796S_MADCTL);	// Memory Data Access Control
	WriteDataSPI(0x68);					// MV   - m.j. i RGB/BGR data order

	WriteCommandSPI(ST7796S_DIC);		// Display Inversion Control
	WriteDataSPI(0x01);

	WriteCommandSPI(ST7796S_CASET);		// Column address set
	WriteDataSPI(0x00);
	WriteDataSPI(0x00);
	WriteDataSPI(0x01);
	WriteDataSPI(0xDF);

	WriteCommandSPI(ST7796S_RASET);		// Column address set
	WriteDataSPI(0x00);
	WriteDataSPI(0x00);
	WriteDataSPI(0x01);
	WriteDataSPI(0x3F);

	WriteCommandSPI(ST7796S_DFC);		// Display Function Control
	WriteDataSPI(0x00);					// nebude prozatim RGB interface
	WriteDataSPI(0x22);					// SS=1  ISC=02
	WriteDataSPI(0x3B);					// 3B- 59 - 60*8 480  nebude prozatim RGB interface

	WriteCommandSPI(ST7796S_DOCA);		// Display Output Ctrl Adjust
	WriteDataSPI(0x40);					//
	WriteDataSPI(0x8A);					//
	WriteDataSPI(0x00);					//
	WriteDataSPI(0x00);					//

	WriteDataSPI(0x25);					// Source timing Control(us) S_END[3:0] 0 - f, S_END * 1.5us + 9us  9~22.5us
	WriteDataSPI(0x0a);					// G_START[5:0] - defaultne 0Ah
	WriteDataSPI(0x38);					// G_END[5:0] - defaultne 38h
	WriteDataSPI(0x33);					//

	WriteCommandSPI(ST7796S_PWCTR2);	// Power Control 2
	WriteDataSPI(0x06);					// VRH[6:0]

	WriteCommandSPI(ST7796S_PWCTR3);	// Power Control 3
	WriteDataSPI(0xA7);					// Source/Gamma driving current level

	WriteCommandSPI(ST7796S_VCMPCTL);	// VCOM Control
	WriteDataSPI(0x18);					// default 1ch VCMP[5:0]  0.9

	WriteCommandSPI(ST7796S_WRCABC);	// VCOM Control
	WriteDataSPI(0x93);					// default 1ch VCMP[5:0]  0.9


	WriteCommandSPI(ST7796S_PGC);		// Positive Gamma Control
	WriteDataSPI(0xF0);					// 1
	WriteDataSPI(0x09);					// 2
	WriteDataSPI(0x0b);					// 3
	WriteDataSPI(0x06);					// 4
	WriteDataSPI(0x04);					// 5
	WriteDataSPI(0x15);					// 6
	WriteDataSPI(0x2f);					// 7
	WriteDataSPI(0x54);					// 8
	WriteDataSPI(0x42);					// 9
	WriteDataSPI(0x3c);					// 10
	WriteDataSPI(0x17);					// 11
	WriteDataSPI(0x14);					// 12
	WriteDataSPI(0x18);					// 13
	WriteDataSPI(0x1b);					// 14

	WriteCommandSPI(ST7796S_NGC);		// Positive Gamma Control
	WriteDataSPI(0xF0);					// 1
	WriteDataSPI(0x09);					// 2
	WriteDataSPI(0x0b);					// 3
	WriteDataSPI(0x06);					// 4
	WriteDataSPI(0x04);					// 5
	WriteDataSPI(0x03);					// 6
	WriteDataSPI(0x2d);					// 7
	WriteDataSPI(0x43);					// 8
	WriteDataSPI(0x42);					// 9
	WriteDataSPI(0x3b);					// 10
	WriteDataSPI(0x16);					// 11
	WriteDataSPI(0x14);					// 12
	WriteDataSPI(0x17);					// 13
	WriteDataSPI(0x1b);					// 14

	WriteCommandSPI(ST7796S_COLMOD);
	WriteDataSPI(0x05);

	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
	WriteDataSPI(0x3c);					// 3c disable command 2 part I

	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
	WriteDataSPI(0x69);					// 69 disable  command 2 part II

	MAP_SysCtlDelay(20 * ui32ClockMS);
	MAP_SysCtlDelay(20 * ui32ClockMS);

	WriteCommandSPI(ST7796S_DISPON);

	//
	// Switch on the LED backlight
	//
	LEDBacklightON();
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
	MAP_SysCtlDelay(10 * ui32ClockMS);

	// Reset the LCD
	GPIOPinWrite(LCD_RST_BASE, LCD_RST_PIN, 0);
	MAP_SysCtlDelay(400 * ui32ClockMS);


	// Reset RELEASE
	GPIOPinWrite(LCD_RST_BASE, LCD_RST_PIN, LCD_RST_PIN);
	MAP_SysCtlDelay(400 * ui32ClockMS);

	WriteCommandSPI(ST7796S_SWRESET);
	MAP_SysCtlDelay(120 * ui32ClockMS);

	// enable commands 2
	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
	WriteDataSPI(0xc3);					// D[7:0] = C3h enable command 2 part I
	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
	WriteDataSPI(0x96);					// D[7:0] = C3h enable command 2 part II

	WriteCommandSPI(ST7796S_SLPOUT);
	MAP_SysCtlDelay(120 * ui32ClockMS);

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

	MAP_SysCtlDelay(80 * ui32ClockMS);

	WriteCommandSPI(ST7796S_DISPON);
	MAP_SysCtlDelay(60 * ui32ClockMS);

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


void Reconfigure(void) {
	static uint8_t command = 0x36;		//ST7796S_MADCTL
	static uint8_t pp = 1;				// pocet parametru
	static uint8_t x = 0;
	static uint8_t dato1 = 0x00;
	static uint8_t dato2 = 0x22;
	static uint8_t dato3 = 0x3b;
	static uint8_t dato4 = 0;

		WriteCommandSPI(command);		// Command Set Control
		x = pp;
		if (x) {
			WriteDataSPI(dato1);
			x--;
		}
		if (x) {
			WriteDataSPI(dato2);
			x--;
		}
		if (x) {
			WriteDataSPI(dato3);
			x--;
		}
		if (x) {
			WriteDataSPI(dato4);
			x--;
		}
}


// void Powertip320x480x16_ToRGB(void) {
//
// 	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
// 	WriteDataSPI(0xc3);					// D[7:0] = C3h enable command 2 part I
//
// 	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
// 	WriteDataSPI(0x96);					// D[7:0] = C3h enable command 2 part II
//
// 	WriteCommandSPI(ST7796S_IFMODE);	// Display Function Control
// 	WriteDataSPI(0x00);
//
//
// 	WriteCommandSPI(ST7796S_DFC);		// Display Function Control
// // 	WriteDataSPI(0xe0);					// RGB interface  + SYNC_MODE
// 	WriteDataSPI(0xA0);					// RGB interface  + SYNC_MODE
// 	WriteDataSPI(0x22);					// SS=1  ISC=02
// 	WriteDataSPI(0x3B);					// 3B- 59 - 60*8 480
//
// 	WriteCommandSPI(ST7796S_BPC);		// Display Function Control
// 	WriteDataSPI(g_sPowertip480x320x60Hz.sTiming.ui8VFrontPorch);
// 	WriteDataSPI(g_sPowertip480x320x60Hz.sTiming.ui8VBackPorch);
// 	WriteDataSPI(0x00);
// 	WriteDataSPI(((g_sPowertip480x320x60Hz.sTiming.ui16HBackPorch) & 0xFF));
//
// 	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
// 	WriteDataSPI(0x3c);					// 3c disable command 2 part I
//
// 	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
// 	WriteDataSPI(0x69);					// 69 disable  command 2 part II
// }
//
// void Powertip320x480x16_ToSPI(void) {
// 	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
// 	WriteDataSPI(0xc3);					// D[7:0] = C3h enable command 2 part I
//
// 	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
// 	WriteDataSPI(0x96);					// D[7:0] = C3h enable command 2 part II
//
//
//
// 	WriteCommandSPI(ST7796S_DFC);		// Display Function Control
// 	WriteDataSPI(0x00);					// nebude prozatim RGB interface
// 	WriteDataSPI(0x22);					// SS=1  ISC=02
// 	WriteDataSPI(0x3B);					// 3B- 59 - 60*8 480  nebude prozatim RGB interface
//
//
//
// 	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
// 	WriteDataSPI(0x3c);					// 3c disable command 2 part I
//
// 	WriteCommandSPI(ST7796S_CSCON);		// Command Set Control
// 	WriteDataSPI(0x69);					// 69 disable  command 2 part II
// }

//*****************************************************************************
//
//! Draws a pixel on the screen.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param i32X is the X coordinate of the pixel.
//! \param i32Y is the Y coordinate of the pixel.
//! \param ui32Value is the color of the pixel.
//!
//! This function sets the given pixel to a particular color.  The coordinates
//! of the pixel are assumed to be within the extents of the display.
//!
//! \return None.
//
//*****************************************************************************
static void Powertip320x480x16_ST7796SPixelDraw(__attribute__((unused)) void *pvDisplayData, int32_t i32X,  int32_t i32Y, uint32_t ui32Value) {
	WriteCommandSPI(ST7796S_CASET);		// Column address set
	WriteData16SPI(i32X);
	WriteData16SPI((i32X + 1));

	WriteCommandSPI(ST7796S_RASET);		// Column address set
	WriteData16SPI(i32Y);
	WriteData16SPI((i32Y + 1));

	WriteCommandSPI(ST7796S_RAMWR);
	WriteData16SPI(DPYCOLORTRANSLATE(ui32Value));
}

//*****************************************************************************
//
//! Draws a horizontal sequence of pixels on the screen.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param i32X is the X coordinate of the first pixel.
//! \param i32Y is the Y coordinate of the first pixel.
//! \param i32X0 is sub-pixel offset within the pixel data, which is valid for
//! 1 or 4 bit per pixel formats.
//! \param i32Count is the number of pixels to draw.
//! \param i32BPP is the number of bits per pixel; must be 1, 4, or 8.
//! \param pui8Data is a pointer to the pixel data.  For 1 and 4 bit per pixel
//! formats, the most significant bit(s) represent the left-most pixel.
//! \param pui8Palette is a pointer to the palette used to draw the pixels.
//!
//! This function draws a horizontal sequence of pixels on the screen, using
//! the supplied palette.  For 1 bit per pixel format, the palette contains
//! pre-translated colors; for 4 and 8 bit per pixel formats, the palette
//! contains 24-bit RGB values that must be translated before being written to
//! the display.
//!
//! \return None.
//
//*****************************************************************************
static void Powertip320x480x16_ST7796SPixelDrawMultiple(void *pvDisplayData, int32_t i32X,
        int32_t i32Y, int32_t i32X0,
        int32_t i32Count, int32_t i32BPP,
        const uint8_t *pui8Data,
        const uint8_t *pui8Palette) {
	uint32_t ui32Byte;

	// Set the starting X address of the display cursor.
	WriteCommandSPI(ST7796S_CASET);		// Column address set
	WriteData16SPI(i32X);
	WriteData16SPI((i32X + i32Count));

	// Set the Y address of the display cursor.
	WriteCommandSPI(ST7796S_RASET);		// Column address set
	WriteData16SPI(i32Y);
	WriteData16SPI((i32Y + 1));

	// Write the data RAM write command.
	WriteCommandSPI(ST7796S_RAMWR);


	//
	// Determine how to interpret the pixel data based on the number of bits
	// per pixel.
	//
	switch (i32BPP & ~GRLIB_DRIVER_FLAG_NEW_IMAGE) {
		//
		// The pixel data is in 1 bit per pixel format.
		//
		case 1: {
			// Loop while there are more pixels to draw.
			while (i32Count) {
				// Get the next byte of image data.
				ui32Byte = *pui8Data++;

				// Loop through the pixels in this byte of image data.
				for (; (i32X0 < 8) && i32Count; i32X0++, i32Count--) {
					// Draw this pixel in the appropriate color.
					WriteData16SPI(((uint32_t *)pui8Palette)[(ui32Byte >> (7 - i32X0)) & 1]);
				}
				// Start at the beginning of the next byte of image data.
				i32X0 = 0;
			}
			// The image data has been drawn.
			break;
		}

		// The pixel data is in 4 bit per pixel format.
		case 4: {
			//
			// Loop while there are more pixels to draw.  "Duff's device" is
			// used to jump into the middle of the loop if the first nibble of
			// the pixel data should not be used.  Duff's device makes use of
			// the fact that a case statement is legal anywhere within a
			// sub-block of a switch statement.  See
			// http://en.wikipedia.org/wiki/Duff's_device for detailed
			// information about Duff's device.
			//
			switch (i32X0 & 1) {
				case 0:
					while (i32Count) {
						// Get the upper nibble of the next byte of pixel data
						// and extract the corresponding entry from the palette.
						ui32Byte = (*pui8Data >> 4) * 3;
						ui32Byte = (*(uint32_t *)(pui8Palette + ui32Byte) & 0x00ffffff);

						// Translate this palette entry and write it to the screen.
						WriteData16SPI(DPYCOLORTRANSLATE(ui32Byte));

						// Decrement the count of pixels to draw.
						i32Count--;

						// See if there is another pixel to draw.
						if (i32Count) {
						case 1:
							// Get the lower nibble of the next byte of pixel
							// data and extract the corresponding entry from
							// the palette.
							ui32Byte = (*pui8Data++ & 15) * 3;
							ui32Byte = (*(uint32_t *)(pui8Palette + ui32Byte) & 0x00ffffff);

							// Translate this palette entry and write it to the screen.
							WriteData16SPI(DPYCOLORTRANSLATE(ui32Byte));

							// Decrement the count of pixels to draw.
							i32Count--;
						}
					}
			}
			// The image data has been drawn.
			break;
		}
		// The pixel data is in 8 bit per pixel format.
		case 8: {
			// Loop while there are more pixels to draw.
			while (i32Count--) {
				// Get the next byte of pixel data and extract the
				// corresponding entry from the palette.
				ui32Byte = *pui8Data++ * 3;
				ui32Byte = *(uint32_t *)(pui8Palette + ui32Byte) & 0x00ffffff;
				// Translate this palette entry and write it to the screen.
				WriteData16SPI(DPYCOLORTRANSLATE(ui32Byte));
			}
			// The image data has been drawn.
			break;
		}
		// We are being passed data in the display's native format.  Merely
		// write it directly to the display.  This is a special case which is
		// not used by the graphics library but which is helpful to
		// applications which may want to handle, for example, JPEG images.
		case 16: {
			uint16_t ui16Byte;
			// Loop while there are more pixels to draw.
			while (i32Count--) {
				// Get the next byte of pixel data and extract the
				// corresponding entry from the palette.
				ui16Byte = *((uint16_t *)pui8Data);
				pui8Data += 2;
				// Translate this palette entry and write it to the screen.
				WriteData16SPI(ui16Byte);
			}
		}
	}
}

//*****************************************************************************
//
//! Draws a horizontal line.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param i32X1 is the X coordinate of the start of the line.
//! \param i32X2 is the X coordinate of the end of the line.
//! \param i32Y is the Y coordinate of the line.
//! \param ui32Value is the color of the line.
//!
//! This function draws a horizontal line on the display.  The coordinates of
//! the line are assumed to be within the extents of the display.
//!
//! \return None.
//
//*****************************************************************************
static void
Powertip320x480x16_ST7796SLineDrawH(__attribute__((unused))  void *pvDisplayData, int32_t i32X1,
                                    int32_t i32X2, int32_t i32Y,
                                    uint32_t ui32Value) {
	// Set the starting X address of the display cursor.
	WriteCommandSPI(ST7796S_CASET);		// Column address set
	WriteData16SPI(i32X1);
	WriteData16SPI((i32X2));

	// Set the Y address of the display cursor.
	WriteCommandSPI(ST7796S_RASET);		// Column address set
	WriteData16SPI(i32Y);
	WriteData16SPI((i32Y));

	//
	// Write the data RAM write command.
	//
	WriteCommandSPI(ST7796S_RAMWR);

	ui32Value = DPYCOLORTRANSLATE(ui32Value);
	//
	// Loop through the pixels of this horizontal line.
	//
	while (i32X1++ <= i32X2) {
		// Write the pixel value.
		WriteData16SPI(ui32Value);
	}
}

//*****************************************************************************
//
//! Draws a vertical line.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param i32X is the X coordinate of the line.
//! \param i32Y1 is the Y coordinate of the start of the line.
//! \param i32Y2 is the Y coordinate of the end of the line.
//! \param ui32Value is the color of the line.
//!
//! This function draws a vertical line on the display.  The coordinates of the
//! line are assumed to be within the extents of the display.
//!
//! \return None.
//
//*****************************************************************************
static void
Powertip320x480x16_ST7796SLineDrawV(__attribute__((unused))  void *pvDisplayData, int32_t i32X,
                                    int32_t i32Y1, int32_t i32Y2,
                                    uint32_t ui32Value) {
	// Set the X address of the display cursor.
	WriteCommandSPI(ST7796S_CASET);		// Column address set
	WriteData16SPI(i32X);
	WriteData16SPI((i32X));

	// Set the starting Y address of the display cursor.
	WriteCommandSPI(ST7796S_RASET);		// Column address set
	WriteData16SPI(i32Y1);
	WriteData16SPI((i32Y2));

	// Write the data RAM write command.
	WriteCommandSPI(ST7796S_RAMWR);

	ui32Value = DPYCOLORTRANSLATE(ui32Value);

	// Loop through the pixels of this vertical line.
	while (i32Y1++ <= i32Y2) {
		// Write the pixel value.
		WriteData16SPI(ui32Value);
	}
}

//*****************************************************************************
//
//! Fills a rectangle.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param pRect is a pointer to the structure describing the rectangle.
//! \param ui32Value is the color of the rectangle.
//!
//! This function fills a rectangle on the display.  The coordinates of the
//! rectangle are assumed to be within the extents of the display, and the
//! rectangle specification is fully inclusive (in other words, both sXMin and
//! sXMax are drawn, along with sYMin and sYMax).
//!
//! \return None.
//
//*****************************************************************************
static void Powertip320x480x16_ST7796SRectFill(__attribute__((unused)) void *pvDisplayData, const tRectangle *pRect, uint32_t ui32Value) {
	int32_t i32Count;

	// Write the X extents of the rectangle.
	WriteCommandSPI(ST7796S_CASET);		// Column address set
	WriteData16SPI(pRect->i16XMin);
	WriteData16SPI(pRect->i16XMax);

	// Write the Y extents of the rectangle.
	WriteCommandSPI(ST7796S_RASET);		// Column address set
	WriteData16SPI(pRect->i16YMin);
	WriteData16SPI(pRect->i16YMax);

	// Tell the controller we are about to write data into its RAM.
	WriteCommandSPI(ST7796S_RAMWR);
	ui32Value = DPYCOLORTRANSLATE(ui32Value);

	// Loop through the pixels of this filled rectangle.
	for (i32Count = ((pRect->i16XMax - pRect->i16XMin + 1) *
	                 (pRect->i16YMax - pRect->i16YMin + 1));
	        i32Count >= 0; i32Count--) {
		// Write the pixel value.
		WriteData16SPI(ui32Value);
	}


//     // Reset the X extents to the entire screen.
//     WriteCommandSPI(ST7796S_CASET);		// Column address set
// 	WriteData16SPI(0);
// 	WriteData16SPI(479);
//
// 	// Reset the Y extent to the full screen
// 	WriteCommandSPI(ST7796S_RASET);		// Column address set
// 	WriteData16SPI(0);
// 	WriteData16SPI(319);
}

//*****************************************************************************
//
//! Translates a 24-bit RGB color to a display driver-specific color.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//! \param ui32Value is the 24-bit RGB color.  The least-significant byte is
//! the blue channel, the next byte is the green channel, and the third byte is
//! the red channel.
//!
//! This function translates a 24-bit RGB color into a value that can be
//! written into the display's frame buffer in order to reproduce that color,
//! or the closest possible approximation of that color.
//!
//! \return Returns the display-driver specific color.
//
//*****************************************************************************
static uint32_t
Powertip320x480x16_ST7796SColorTranslate(void *pvDisplayData,
        uint32_t ui32Value) {
	//
	// Translate from a 24-bit RGB color to a 5-6-5 RGB color.
	//
	return (DPYCOLORTRANSLATE(ui32Value));
}

//*****************************************************************************
//
//! Flushes any cached drawing operations.
//!
//! \param pvDisplayData is a pointer to the driver-specific data for this
//! display driver.
//!
//! This functions flushes any cached drawing operations to the display.  This
//! is useful when a local frame buffer is used for drawing operations, and the
//! flush would copy the local frame buffer to the display.  For the SSD2119
//! driver, the flush is a no operation.
//!
//! \return None.
//
//*****************************************************************************
static void
Powertip320x480x16_ST7796SFlush(void *pvDisplayData __attribute__((unused))) {
	//
	// There is nothing to be done.
	//
}

//*****************************************************************************
//
//! The display structure that describes the driver for the Kentec
//! K350QVG-V2-F TFT panel with an SSD2119 controller.
//
//*****************************************************************************
const tDisplay g_sPowertip320x480x16_ST7796S = {
	sizeof(tDisplay),
	0,
/*#if defined(PORTRAIT) || defined(PORTRAIT_FLIP)
	LCD_VERTICAL_MAX,
	LCD_HORIZONTAL_MAX,
#else
	LCD_VERTICAL_MAX,
	LCD_HORIZONTAL_MAX,
#endif
	*/
	#if (defined NORMAL) || (defined DISPLAY_ROTATE_180)
	RASTER_WIDTH,
	RASTER_HEIGHT,
	#else
	RASTER_HEIGHT,
	RASTER_WIDTH,
	#endif


// 	LCD_HORIZONTAL_MAX,
// 	LCD_VERTICAL_MAX,
	Powertip320x480x16_ST7796SPixelDraw,
	Powertip320x480x16_ST7796SPixelDrawMultiple,
	Powertip320x480x16_ST7796SLineDrawH,
	Powertip320x480x16_ST7796SLineDrawV,
	Powertip320x480x16_ST7796SRectFill,
	Powertip320x480x16_ST7796SColorTranslate,
	Powertip320x480x16_ST7796SFlush
};


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
