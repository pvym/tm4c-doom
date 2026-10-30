#include "loading_screen.h"

#include <stdbool.h>
#include <stdint.h>

#include "config.h"
#include "lcd_init.h"

#define SCREEN_W 480U
#define SCREEN_H 320U

/*
 * Obrázek je na šířku 480x320. Rastr panelu je nativně 320x480, proto se
 * logický pixel (x, y) zapisuje na řádek x, sloupec (319 - y) - stejně jako
 * otočení v DG_DrawFrame.
 */
void LoadingScreen_Show(void)
{
    volatile uint16_t *fb = LCD_GetFramebuffer();
    volatile uint16_t *fb2 = (volatile uint16_t *)(uintptr_t)TM4C_LCD_FRAMEBUFFER2_ADDR;
    uint32_t pos = 0U;

    for (uint32_t i = 0U; i < g_loading_rle_len; i += 2U)
    {
        const uint16_t color = g_loading_palette[g_loading_rle[i + 1U]];

        for (uint32_t n = g_loading_rle[i]; n != 0U; --n, ++pos)
        {
            const uint32_t x = pos % SCREEN_W;
            const uint32_t y = pos / SCREEN_W;

            fb[(x * SCREEN_H) + (SCREEN_H - 1U - y)] = color;
        }
    }

    // Back buffer musí obsahovat totéž, aby po prvním snímku Dooma zůstal rámeček.
    for (uint32_t i = 0U; i < (SCREEN_W * SCREEN_H); ++i)
    {
        fb2[i] = fb[i];
    }
}
