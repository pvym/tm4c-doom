//*****************************************************************************
//
// Kentec320x240x16_ssd2119_spi.h - Prototypes fpr the Kentec
//                                  BOOSTXL-K350QVG-S1 TFT display drivers with
//                                  an SSD2119 and SPI interface.
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

#ifndef __POWERTIP320x480x16_ST7796S_8BIT_H__
#define __POWERTIP320x480x16_ST7796S_8BIT_H__

//*****************************************************************************
//
// Defines for the SSI controller and pins that are used to communicate with
// the SSD2119.
//
//*****************************************************************************

#define LCD_SSI_CLK_PIN         GPIO_PIN_2
#define LCD_SSI_FSS_PIN			GPIO_PIN_3
#define LCD_SSI_TX_PIN          GPIO_PIN_4
#define LCD_SSI_RX_PIN          GPIO_PIN_5

// #define LCD_CS_PERIPH           SYSCTL_PERIPH_GPIOA
// #define LCD_CS_BASE             GPIO_PORTA_BASE
// #define LCD_CS_PIN              GPIO_PIN_3



//*****************************************************************************
//
// Translates a 24-bit RGB color to a display driver-specific color.
//
// \param c is the 24-bit RGB color.  The least-significant byte is the blue
// channel, the next byte is the green channel, and the third byte is the red
// channel.
//
// This macro translates a 24-bit RGB color into a value that can be written
// into the display's frame buffer in order to reproduce that color, or the
// closest possible approximation of that color.
//
// \return Returns the display-driver specific color.
//
//*****************************************************************************
#define DPYCOLORTRANSLATE(c)    ((((c) & 0x00f80000) >> 8) |               \
                                 (((c) & 0x0000fc00) >> 5) |               \
                                 (((c) & 0x000000f8) >> 3))

//*****************************************************************************


//*****************************************************************************
//
// Prototypes for the globals exported by this driver.
//
//*****************************************************************************
void LEDBacklightON(void);
void LEDBacklightOFF(void);
void InitLCDGpioForSPI(void);
void InitLCDGpioForRGBSPI(void);
void InitST7796S_RGB(uint32_t ui32SysClock);
void Powertip320x480x16_ST7796SInit(uint32_t ui32SysClock);
void Powertip320x480x16_ToRGB(void);
void Powertip320x480x16_ToSPI(void);
void Reconfigure(void);

extern const tDisplay g_sPowertip320x480x16_ST7796S;

#endif // __POWERTIP320x480x16_ST7796S_H__
