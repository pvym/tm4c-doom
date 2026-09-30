//*****************************************************************************
//
// sram_startup_gcc.S - Startup code for GNU.
//
// Copyright (c) 2022 Pavel Vymetalek
//
//
//*****************************************************************************

//*****************************************************************************
//
// Include the assember definitions used to make this code compiler
// independent.
//
//*****************************************************************************
#include "inc/hw_nvic.h"
#include "inc/hw_sysctl.h"

//*****************************************************************************
//
// Put the assembler into the correct configuration.
//
//*****************************************************************************
    .syntax unified
    .thumb

//*****************************************************************************
//
// The stack gets placed into the zero-init section.
//
//*****************************************************************************
    .bss



//*****************************************************************************
//
// This portion of the file goes into the text section.
//
//*****************************************************************************
    .text

//*****************************************************************************
//
// * Inicializace NVIC_VTABLE - vektory presunout do SRAM
// * Nastavit vrchol zasobniku - registr SP na hodnotu z prvniho dwordu tabulky vektoru,
//   ktera ukazuje na oblast zasobniku [256]
// * Pak se vezme adresa ResetISR programu ve SRAM a zavola se
//
//*****************************************************************************
    .thumb_func
    .globl SramExecInit
SramExecInit:
    //
    // Set the vector table pointer to the beginning of SRAM.
    //
    movw    r0, #(NVIC_VTABLE & 0xffff)
    movt    r0, #(NVIC_VTABLE >> 16)
    movs    r1, #0x0000
    movt    r1, #0x2000
    str     r1, [r0]

    //
    // Load the stack pointer from the application's vector table.
    //
    movs    r0, #0x0000
    movt    r0, #0x2000
    ldr     sp, [r0]

	// vezme adresu ResetISR - reset vektoru a skoci tam
    movs    r1, #0x0004
    movt    r1, #0x2000
	ldr r0, [r1]
	bx r0
