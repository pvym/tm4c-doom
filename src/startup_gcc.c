//*****************************************************************************
//
// startup_gcc.c - Local startup code for GNU tools.
//
// Derived from the TM4C startup used in x-v2 and TivaWare examples.
//
//*****************************************************************************

#include <stdint.h>

#include "inc/hw_nvic.h"
#include "inc/hw_types.h"

void ResetISR(void);
static void NmiSR(void);
static void FaultISR(void);
static void IntDefaultHandler(void);

extern int main(void);
extern void SysTick_Handler(void);
extern uint32_t _estack;

__attribute__ ((section(".isr_vector")))
void (* const g_pfnVectors[])(void) =
{
    (void (*)(void))(&_estack),
    ResetISR,
    NmiSR,
    FaultISR,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    0,
    0,
    0,
    0,
    IntDefaultHandler,
    IntDefaultHandler,
    0,
    IntDefaultHandler,
    SysTick_Handler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    0,
    0,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    0,
    0,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    0,
    0,
    0,
    0,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    0,
    0,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    0,
    0,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler,
    IntDefaultHandler
};

extern uint32_t _ldata;
extern uint32_t _data;
extern uint32_t _edata;
extern uint32_t _bss;
extern uint32_t _ebss;

void
ResetISR(void)
{
    uint32_t *pui32Src;
    uint32_t *pui32Dest;

    pui32Src = &_ldata;
    for (pui32Dest = &_data; pui32Dest < &_edata; )
    {
        *pui32Dest++ = *pui32Src++;
    }

    __asm("    ldr     r0, =_bss\n"
          "    ldr     r1, =_ebss\n"
          "    mov     r2, #0\n"
          "    .thumb_func\n"
          "zero_loop:\n"
          "        cmp     r0, r1\n"
          "        it      lt\n"
          "        strlt   r2, [r0], #4\n"
          "        blt     zero_loop");

    HWREG(NVIC_CPAC) = ((HWREG(NVIC_CPAC) &
                         ~(NVIC_CPAC_CP10_M | NVIC_CPAC_CP11_M)) |
                        NVIC_CPAC_CP10_FULL | NVIC_CPAC_CP11_FULL);

    main();
}

static void
NmiSR(void)
{
    while (1)
    {
    }
}

extern volatile uint16_t *LCD_GetFramebuffer(void);

// Font 3x5, řádky po 3 bitech, horní řádek nejvýše.
static const uint16_t g_fault_font[16] =
{
    0x7B6F, 0x2C97, 0x73E7, 0x73CF, 0x5BC9, 0x79CF, 0x79EF, 0x7292,
    0x7BEF, 0x7BCF, 0x2BED, 0x6BAE, 0x7247, 0x6B6E, 0x79E7, 0x79E4
};

static void
FaultDrawWord(uint32_t value, uint32_t y_start, uint16_t color)
{
    volatile uint16_t *fb = LCD_GetFramebuffer();

    for (uint32_t d = 0; d < 8; ++d)
    {
        uint16_t glyph = g_fault_font[(value >> (28 - (4 * d))) & 0xF];

        for (uint32_t row = 0; row < 5; ++row)
        {
            for (uint32_t col = 0; col < 3; ++col)
            {
                uint16_t c = (glyph & (1U << (14 - ((row * 3) + col)))) ? color : 0x0000;

                for (uint32_t y = 0; y < 8; ++y)
                {
                    for (uint32_t x = 0; x < 8; ++x)
                    {
                        fb[((y_start + (row * 8) + y) * 320) + (d * 36) + (col * 8) + x] = c;
                    }
                }
            }
        }
    }
}

void
FaultReport(uint32_t *stack)
{
    volatile uint16_t *fb = LCD_GetFramebuffer();

    for (uint32_t i = 0; i < (320UL * 480UL); ++i)
    {
        fb[i] = 0xF800;
    }

    FaultDrawWord(stack[6], 20, 0xFFFF);
    FaultDrawWord(stack[5], 80, 0xFFE0);
    FaultDrawWord(HWREG(NVIC_FAULT_STAT), 140, 0x07FF);
    FaultDrawWord(HWREG(NVIC_FAULT_ADDR), 200, 0x07E0);
    FaultDrawWord(stack[1], 260, 0xF81F);
    FaultDrawWord(stack[3], 320, 0xFFFF);
    FaultDrawWord(stack[0], 380, 0xFFE0);

    while (1)
    {
    }
}

static void __attribute__((naked))
FaultISR(void)
{
    __asm volatile(
        "tst lr, #4\n"
        "ite eq\n"
        "mrseq r0, msp\n"
        "mrsne r0, psp\n"
        "b FaultReport\n");
}

static void
IntDefaultHandler(void)
{
    while (1)
    {
    }
}
