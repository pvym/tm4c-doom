/*
 * tm4c_sdram.h - inicializace externí SDRAM přes EPI (TM4C129, 16bit SDRAM).
 *
 * Pinout EPI (EPI0S0..S31) odpovídá desce s SDRAM na portech
 * A, B, C, G, K, L, M, N (viz tabulka v tm4c_sdram.c). Po úspěšné inicializaci
 * je SDRAM namapována od TM4C_SDRAM_BASE.
 *
 * Příklad:
 *   uint32_t sysclk = SysCtlClockFreqSet(..., 120000000);
 *   if (!Tm4cSdramInit(sysclk, TM4C_SDRAM_DEFAULT_REFRESH)) { ... }
 *   volatile uint16_t *ram = (volatile uint16_t *)TM4C_SDRAM_BASE;
 */
#ifndef TM4C_SDRAM_H
#define TM4C_SDRAM_H

#include <stdbool.h>
#include <stdint.h>

#define TM4C_SDRAM_BASE            0x60000000UL
#define TM4C_SDRAM_DEFAULT_REFRESH 400U /* taktů EPI; 256Mbit SDRAM => max 7,8 us */

/*
 * sysclk_hz: systémové hodiny (EPI běží na sysclk/2 nad 60 MHz).
 * refresh_clocks: perioda refreshe v taktech EPI.
 * Vrací false, pokud nedoběhla inicializační sekvence SDRAM.
 */
bool Tm4cSdramInit(uint32_t sysclk_hz, uint16_t refresh_clocks);

/* Jednoduchý test: zapíše/ověří 'words' 16bitových slov od adresy base. */
bool Tm4cSdramTest(volatile uint16_t *base, uint32_t words);

#endif
