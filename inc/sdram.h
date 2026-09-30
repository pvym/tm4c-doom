#ifndef __SDRAM_H__
#define __SDRAM_H__


//*****************************************************************************
//
// Use the following to specify the GPIO pins used by the SDRAM EPI bus.
//
//*****************************************************************************
#define EPI_PORTA_PINS (GPIO_PIN_7 | GPIO_PIN_6)
#define EPI_PORTB_PINS (GPIO_PIN_3)
#define EPI_PORTC_PINS (GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4)
#define EPI_PORTG_PINS (GPIO_PIN_1 | GPIO_PIN_0)
#define EPI_PORTK_PINS (GPIO_PIN_5 | GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0)
#define EPI_PORTL_PINS (GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0)
#define EPI_PORTM_PINS (GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1 | GPIO_PIN_0)
#define EPI_PORTN_PINS (GPIO_PIN_3 | GPIO_PIN_2)

//*****************************************************************************
//
// The starting and ending address for the 32MB SDRAM chip (32Meg x 16bits) on
// the SDRAM daughter board.
//
//*****************************************************************************
#define SDRAM_START_ADDRESS 0x00000000
#define SDRAM_END_ADDRESS   0x00FFFFFF
// #define SDRAM_END_ADDRESS   0x07FFFFFF

//*****************************************************************************
//
// The Mapping address space for the EPI SDRAM.
//
//*****************************************************************************
#define SDRAM_MAPPING_ADDRESS 0x10000000

//*****************************************************************************
//
// A pointer to the EPI memory aperture.  Note that g_pui16EPISdram is declared
// as volatile so the compiler should not optimize reads out of the image.
//
//*****************************************************************************
// static volatile uint16_t *g_pui16EPISdram;

#define NO_OF_RAM_LOC		4096

//*****************************************************************************
//
// The Max Count value for the SysTick Timer.
//
//*****************************************************************************
#define SYSTICK_MAX_COUNT	16777216


void InitSDRAM(void);
void TestSDRAM(void);

#endif // __SDRAM_H__
