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

static void
FaultISR(void)
{
    while (1)
    {
    }
}

static void
IntDefaultHandler(void)
{
    while (1)
    {
    }
}
