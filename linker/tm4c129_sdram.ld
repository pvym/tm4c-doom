/******************************************************************************
 *
 * fl380-v2.ld - Linker configuration file for FL380.
 *
 * Copyright (c) 2013-2017 Texas Instruments Incorporated.  All rights reserved.
 * Software License Agreement
 *
 * Texas Instruments (TI) is supplying this software for use solely and
 * exclusively on TI's microcontroller products. The software is owned by
 * TI and/or its suppliers, and is protected under applicable copyright
 * laws. You may not combine this software with "viral" open-source
 * software in order to form a larger program.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
 * NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
 * NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
 * CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 * This is part of revision 2.1.4.178 of the EK-TM4C129XNCZDA Firmware Package.
 *
 *****************************************************************************/

ENTRY(ResetISR)

MEMORY
{
    FLASH      (rx)  : ORIGIN = 0x00000000, LENGTH = 1024K
    SRAM       (rwx) : ORIGIN = 0x20000000, LENGTH = 256K
    SDRAM_ZONE (rwx) : ORIGIN = 0x10050000, LENGTH = 8M
    SDRAM_WAD  (rwx) : ORIGIN = 0x11000000, LENGTH = 7M
    LCD_FB     (rwx) : ORIGIN = 0x10000000, LENGTH = 320K
}

_estack = ORIGIN(SRAM) + LENGTH(SRAM);
PROVIDE(__doom_zone_heap_start__ = ORIGIN(SDRAM_ZONE));
PROVIDE(__doom_zone_heap_end__   = ORIGIN(SDRAM_ZONE) + LENGTH(SDRAM_ZONE));
PROVIDE(__wad_cache_start__      = ORIGIN(SDRAM_WAD));
PROVIDE(__wad_cache_end__        = ORIGIN(SDRAM_WAD) + LENGTH(SDRAM_WAD));
PROVIDE(__lcd_framebuffer_start__ = ORIGIN(LCD_FB));
PROVIDE(__lcd_framebuffer_end__  = ORIGIN(LCD_FB) + LENGTH(LCD_FB));
PROVIDE(__heap_end__             = ORIGIN(SRAM) + LENGTH(SRAM) - 0x1000);
PROVIDE(__stack_limit__          = __heap_end__);
PROVIDE(__stack_top__            = ORIGIN(SRAM) + LENGTH(SRAM));

SECTIONS
{
    .isr_vector ORIGIN(FLASH) :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        _text = .;
        *(.text*)
        *(.rodata*)
        KEEP (*(.init))
        KEEP (*(.fini))
        _etext = .;
    } > FLASH


    .data : AT(ADDR(.text) + SIZEOF(.text))
    {
        _data = .;
        _ldata = LOADADDR (.data);
        *(vtable)
        *(.data*)
        _edata = .;
    } > SRAM

    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > SDRAM_ZONE
    .dummy (DSECT):
    {
        _dummy = .;
        *(.ARM.exidx*)
        _dummy = .;
    }

    .framebuffer (NOLOAD) :
    {
        *(.framebuffer*)
    } > LCD_FB

    ASSERT(ADDR(.isr_vector) == ORIGIN(FLASH), ".isr_vector is not at flash origin");
}

PROVIDE(_ldata = LOADADDR(.data));
PROVIDE(__heap_start__ = _ebss);
