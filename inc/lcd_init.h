#ifndef TM4C_LCD_INIT_H
#define TM4C_LCD_INIT_H

#include <stdint.h>

void LCD_Init(void);
void LCD_Clear(uint16_t color);
volatile uint16_t *LCD_GetFramebuffer(void);

#endif
