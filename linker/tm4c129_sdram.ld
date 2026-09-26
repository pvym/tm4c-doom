ENTRY(ResetISR)

MEMORY
{
    FLASH      (rx)  : ORIGIN = 0x00000000, LENGTH = 1024K
    SRAM       (rwx) : ORIGIN = 0x20000000, LENGTH = 256K
    SDRAM_ZONE (rwx) : ORIGIN = 0x60000000, LENGTH = 16M
    SDRAM_WAD  (rwx) : ORIGIN = 0x61000000, LENGTH = 8M
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
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)
        *(.rodata*)
        *(.glue_7)
        *(.glue_7t)
        *(.eh_frame*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    .data : AT (ADDR(.ARM.exidx) + SIZEOF(.ARM.exidx))
    {
        . = ALIGN(4);
        _data = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _bss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM

    .wad_cache (NOLOAD) :
    {
        *(.wad_cache*)
    } > SDRAM_WAD

    .framebuffer (NOLOAD) :
    {
        *(.framebuffer*)
    } > LCD_FB
}

PROVIDE(_ldata = LOADADDR(.data));
PROVIDE(__heap_start__ = _ebss);
