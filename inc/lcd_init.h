#ifndef TM4C_LCD_INIT_H
#define TM4C_LCD_INIT_H

#include <stdint.h>

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

#define LCD_FRAME_BUFFER_ADDR 0x10000000


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



void LCD_Init(void);
void LCD_Clear(uint16_t color);
volatile uint16_t *LCD_GetFramebuffer(void);


void InitLCDModule(uint32_t ui32SysClkHz);
void InitLCDGpioForRGBSPI(void);
void InitST7796S_RGB(uint32_t ui32SysClock);
void LEDBacklightON(void);
void LEDBacklightOFF(void);


#endif
