#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define LCD_FRAME_BUFFER_ADDR 0x10000000
#include "driverlib/lcd.h"


//*****************************************************************************
//
// Helpful labels relating to the layout of the palette.
//
//*****************************************************************************
#define FB_TYPE_MASK        0x7000
#define FB_TYPE_16BPP       0x4000

#define PAL_RED_MASK        0x0F00
#define PAL_GREEN_MASK      0x00F0
#define PAL_BLUE_MASK       0x000F
#define PAL_RED_SHIFT       8
#define PAL_GREEN_SHIFT     4
#define PAL_BLUE_SHIFT      0

#define GRLIB_RED_MASK      0x00FF0000
#define GRLIB_GREEN_MASK    0x0000FF00
#define GRLIB_BLUE_MASK     0x000000FF
#define GRLIB_RED_SHIFT     16
#define GRLIB_GREEN_SHIFT   8
#define GRLIB_BLUE_SHIFT    0

#define RED_FROM_PAL_ENTRY(x)   (((x) & GRLIB_RED_MASK) >> GRLIB_RED_SHIFT)
#define GREEN_FROM_PAL_ENTRY(x) (((x) & GRLIB_GREEN_MASK) >> GRLIB_GREEN_SHIFT)
#define BLUE_FROM_PAL_ENTRY(x)  (((x) & GRLIB_BLUE_MASK) >> GRLIB_BLUE_SHIFT)

//
// Determine a pixel value from a 24-bit image palette entry.
//
#define PIXEL_FROM_COLOR(c) (((RED_FROM_PAL_ENTRY(c) & 0xF8) << 8) |          \
((GREEN_FROM_PAL_ENTRY(c) & 0xFC) << 3) |        \
((BLUE_FROM_PAL_ENTRY(c) & 0xF8) >> 3))


//RGB565 = (((RGB888 & 0xf80000)>>8) + ((RGB888 & 0xfc00)>>5) + ((RGB888 & 0x00f8)>>3));
#define RGB565(c) (((c & 0xf80000)>>8) + ((c & 0xfc00)>>5) + ((c & 0x00f8)>>3))

extern volatile uint32_t priznak;

//*****************************************************************************
typedef struct
{
	//
	// A text description of the display mode.
	//
	char *pcMode;

	//
	// The required pixel clock for the display mode in Hz.
	//
	uint32_t ui32PixClock;

	//
	// The PLL VCO frequency recommended to allow the desired pixel clock
	// frequency to be achieved using an integer system clock divider.
	//
	uint32_t ui32VCOFrequency;

	//
	// The recommended system clock frequency to set to allow the pixel clock
	// to be achieved using an integer system clock divider.  Other values are
	// possible so this is merely a recommendation.
	//
	uint32_t ui32SysClockFrequency;

	//
	// LCD controller raster display timings for the display mode.
	//
	tLCDRasterTiming sTiming;

	//
	// A pointer to an additional, display-specific function which, of not
	// NULL, must be called to perform display initialization prior to
	// enabling the LCD controller raster engine.
	//
	void (*pfnInitDisplay)(uint32_t);
}
tRasterDisplayInfo;


// orig
#define RASTER_WIDTH 320
#define RASTER_HEIGHT 480



// fyzicke rozmery displeje pri jeho standarni orientaci
#define LCD_VERTICAL_MAX 480
#define LCD_HORIZONTAL_MAX 320

// pro 50Hz pri 10MHz (na 60Hz lze prejit zmenou pixel_clock na 12MHz pri stejnem nastaveni)
#define LCD_PIXEL_CLOCK		10000000		// 10MHz
// #define LCD_HFP				38				// [clocks]
// #define LCD_HBP				24				// [clocks]
// #define LCD_HPW				18				// [clocks]
// #define LCD_VFP				8				// [lines]
// #define LCD_VBP				6				// [lines]
// #define LCD_VS				6				// [lines]
// #define LCD_BIAS_LC			0				// nepouziva se u aktivnich TFT
//
// puvodni nastaveni 60.86Hz pri 10MHz
#define LCD_HFP				10	//38				// [clocks]
#define LCD_HBP				3	//24				// [clocks]
#define LCD_HPW				3	//18				// [clocks]
#define LCD_VFP				3	//8				// [lines]
#define LCD_VBP				3	//6				// [lines]
#define LCD_VS				3	//6				// [lines]
#define LCD_BIAS_LC			0				// nepouziva se u aktivnich TFT


// #define DISPLAY_ROTATE_90
//*****************************************************************************
//
// A couple of macros used to extract the dimensions from an image.
//
//*****************************************************************************
#define IMAGE_WIDTH(ptr) ((*(uint16_t *)((uint8_t *)(ptr) + 1)))
#define IMAGE_HEIGHT(ptr) ((*(uint16_t *)((uint8_t *)(ptr) + 3)))


void LCDIntHandler(void);
// void InitDisplaySPI(void);
void TestDisplay(void);
void InitLCDModule(uint32_t ui32SysClkHz);
void InitGraphics(void);
void TestRasterRender(void);
void ClearFrameBuffer(void);
void ClearContext(tContext *context, bool flush);
// void FadeOutFrameBuffer(void);

void Rotozoomer(void);
void Mandelbrot(void);
void DrawPalette(void);
void Plasma(void);
void Plasma2(void);

// extern const tRasterDisplayInfo g_sInnoLux800x480x60Hz;
extern const tRasterDisplayInfo g_sPowertip480x320x60Hz;


// static uint16_t ZXPALETTE[] = {
// 	RGB565(ClrBlack),
// 	RGB565(ClrBlue),
// 	RGB565(ClrLightBlue),
// 	RGB565(ClrRed),
// 	RGB565(ClrLightPink),
// 	RGB565(ClrMagenta),
// 	RGB565(ClrPurple),
// 	RGB565(ClrGreen),
// 	RGB565(ClrYellow),
// 	RGB565(ClrYellowGreen),
// 	RGB565(ClrCyan),
// 	RGB565(ClrBrown),
// 	RGB565(ClrAzure),
// 	RGB565(ClrDarkGray),
// 	RGB565(ClrGray),
// 	RGB565(ClrWhite)
// };


#endif // __DISPLAY_H__
