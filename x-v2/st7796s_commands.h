#ifndef __ST7796S_COMMANDS_H__
#define __ST7796S_COMMANDS_H__

//*****************************************************************************
//
// Commands ST7796S
//
//*****************************************************************************

#define ST7796S_NOP			0x00	// No operation
#define ST7796S_SWRESET		0x01	// Software Reset
#define ST7796S_RDDID		0x04	// Read Display ID
#define ST7796S_RDDST		0x09	// Read Display Status

#define ST7796S_SLPIN		0x10	// Sleep in
#define ST7796S_SLPOUT		0x11	// Sleep Out
#define ST7796S_PTLON		0x12	// Partial Display Mode On
#define ST7796S_NORON		0x13	// Normal Display Mode On

#define ST7796S_RDDPM		0x0A	// Read Display Power Mode
#define ST7796S_RDDMADCTL	0x0B	// Read Display MADCTL
#define ST7796S_RDDCOLMOD	0x0C	// PIXFMT
#define ST7796S_RDDIM		0x0D	// Read Display Image Mode
#define ST7796S_RDDSM		0x0E	// Read Display Signal Mode
#define ST7796S_RDSELFDIAG	0x0F	// Read Display Self-Diagnostic Result

#define ST7796S_INVOFF		0x20	// Display Inversion Off
#define ST7796S_INVON		0x21	// Display Inversion On
#define ST7796S_GAMMASET	0x26	// FIXME parametr ma ILI9488 ma ho take ST7796S?
#define ST7796S_DISPOFF		0x28	// Display Off
#define ST7796S_DISPON		0x29	// Display On

#define ST7796S_CASET		0x2A	// Column Address Set
#define ST7796S_RASET		0x2B	// Row Address Set
#define ST7796S_RAMWR		0x2C	// Memory Write
#define ST7796S_RAMRD		0x2E	// Memory Read

#define ST7796S_PTLAR		0x30	// Partial Area
#define ST7796S_VSCRDEF		0x33	// Vertical Scrolling Definition
#define ST7796S_TEOFF		0x34	// Tearing Effect Line OFF
#define ST7796S_TEON		0x35	// Tearing Effect Line On
#define ST7796S_MADCTL		0x36	// Memory Data Access Control
#define ST7796S_VSCSAD		0x37	// Vertical Scroll Start Address of RAM
#define ST7796S_IDMOFF		0x38	// Idle Mode Off
#define ST7796S_IDMON		0x39	// Idle mode on
#define ST7796S_COLMOD		0x3A	// Interface Pixel Format
#define ST7796S_WRMEMC		0x3C	// Write Memory Continue
#define ST7796S_RDMEMC		0x3E	// Read Memory Continue

#define ST7796S_WRDISBV		0x51	// Write Display Brightness Value
#define ST7796S_RDDISBV		0x52	// Read Display Brightness Value
#define ST7796S_WRCTRLD		0x53	// Write CTRL Display
#define ST7796S_RDCTRLD		0x54	// Read CTRL value Display
#define ST7796S_WRCABC		0x55	// Write Adaptive Brightness Control
#define ST7796S_RDCABC		0x56	// Read Content Adaptive Brightness Control


#define ST7796S_IFMODE		0xB0	// Interface Mode Control
#define ST7796S_FRMCTR1		0xB1	// Frame Rate Control (In Normal Mode/Full Colors)
#define ST7796S_FRMCTR2		0xB2	// Frame Rate Control 2 (In Idle Mode/8 colors)
#define ST7796S_FRMCTR3		0xB3	// Frame Rate Control3 (In Partial Mode/Full Colors)
#define ST7796S_DIC			0xB4	// Display Inversion Control
#define ST7796S_BPC			0xB5	// Blanking Porch Control
#define ST7796S_DFC			0xB6	// Display Function Control
#define ST7796S_EM			0xB7	// Entry Mode Set

#define ST7796S_PWCTR1		0xC0	// Power Control 1
#define ST7796S_PWCTR2		0xC1	// Power Control 2
#define ST7796S_PWCTR3		0xC2	// Power Control 3
// #define ST7796S_PWCTR4		0xC3	//
// #define ST7796S_PWCTR5		0xC4	//
#define ST7796S_VCMPCTL		0xC5	// VCOM Control
// #define ST7796S_VMCTR2		0xC7	//

#define ST7796S_RDID1		0xDA	// Read ID1
#define ST7796S_RDID2		0xDB	// Read ID2
#define ST7796S_RDID3		0xDC	// Read ID3
#define ST7796S_RDID4		0xD3	// Read ID4

#define ST7796S_PGC			0xE0	// Positive Gamma Control
#define ST7796S_NGC			0xE1	// Negative Gamma Control
#define ST7796S_DOCA		0xE8	// Display Output Ctrl Adjust


#define ST7796S_CSCON		0xF0	// Command Set Control
#define ST7796S_SPIRC		0xFB	// SPI Read Control

#endif	//__ST7796S_COMMANDS_H__
