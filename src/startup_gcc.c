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

// Každý řádek = 32 bitů (MSB vlevo), 10 px na bit: PC, LR, CFSR, BFAR.
extern volatile uint16_t *LCD_GetFramebuffer(void);

static void
FaultDrawWord(uint32_t value, uint32_t y_start, uint16_t one_color)
{
    volatile uint16_t *fb = LCD_GetFramebuffer();

    for (uint32_t bit = 0; bit < 32; ++bit)
    {
        uint16_t color = (value & (0x80000000UL >> bit)) ? one_color : 0x0000;

        for (uint32_t y = y_start; y < (y_start + 30); ++y)
        {
            for (uint32_t x = bit * 10; x < ((bit * 10) + 8); ++x)
            {
                fb[(y * 320) + x] = color;
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
    FaultDrawWord((uint32_t)stack, 260, 0xF81F);

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
