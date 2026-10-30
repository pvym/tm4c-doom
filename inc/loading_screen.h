#ifndef LOADING_SCREEN_H
#define LOADING_SCREEN_H

#include <stdbool.h>
#include <stdint.h>

#define LOADING_SCREEN_COLORS 32U

extern const uint16_t g_loading_palette[LOADING_SCREEN_COLORS];
extern const uint8_t g_loading_rle[];
extern const uint32_t g_loading_rle_len;

/* Vykreslí úvodní obrazovku (480x320 na šířku) do obou framebufferů. */
void LoadingScreen_Show(void);

#endif
