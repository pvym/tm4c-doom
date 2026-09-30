#******************************************************************************
#
# Makefile - Rules for building the FontView example.
#
# Copyright (c) 2012-2017 Texas Instruments Incorporated.  All rights reserved.
# Software License Agreement
#
# Texas Instruments (TI) is supplying this software for use solely and
# exclusively on TI's microcontroller products. The software is owned by
# TI and/or its suppliers, and is protected under applicable copyright
# laws. You may not combine this software with "viral" open-source
# software in order to form a larger program.
#
# THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
# NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
# NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
# CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
# DAMAGES, FOR ANY REASON WHATSOEVER.
#
# This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
#
#******************************************************************************

TARGET_NAME=fl380-v2
TARGET_FILE=${TARGET_NAME}.axf
#
# Defines the part type that this project uses.
#
PART=TM4C129XNCZAD

#
# The base directory for TivaWare.
#
ROOT=../../..

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#
#VPATH=../drivers
VPATH+=${ROOT}/third_party/fonts/ofl/fonts
VPATH+=${ROOT}/utils

#
# Where to find header files that do not live in the source directory.
#
IPATH=..
IPATH+=${ROOT}
IPATH+=${ROOT}/third_party/fonts/ofl
IPATH+=${ROOT}/third_party

CFLAGS+=-specs=nano.specs -specs=nosys.specs -DDISPLAY_ROTATE_90
# -DDISPLAY_ROTATE_270
#-DDISPLAY_ROTATE_90
# -DNORMAL

#
# The default rule, which causes the FontView example to be built.
#
all: ${COMPILER}
all: ${COMPILER}/${TARGET_FILE}

#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} ${wildcard *~}

flash:	all
	@echo "Programming via openocd"
	@echo "Clean all flash - remove bootloader - need fix... write bin file to address..."
	openocd -f ../../../x-openocd/ek-tm4c129xnczad.cfg -c "program ${COMPILER}/${TARGET_NAME}.axf verify reset exit"

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir -p ${COMPILER}

#
# Rules for building the FontView example.
#
${COMPILER}/${TARGET_FILE}: ${COMPILER}/sdram.o
${COMPILER}/${TARGET_FILE}: ${COMPILER}/grlib_raster_driver_16bpp.o
${COMPILER}/${TARGET_FILE}: ${COMPILER}/display.o
${COMPILER}/${TARGET_FILE}: ${COMPILER}/keyb.o
${COMPILER}/${TARGET_FILE}: ${COMPILER}/event.o
${COMPILER}/${TARGET_FILE}: ${COMPILER}/comm.o
${COMPILER}/${TARGET_FILE}: ${COMPILER}/user.o
${COMPILER}/${TARGET_FILE}: ${COMPILER}/test.o
${COMPILER}/${TARGET_FILE}: ${COMPILER}/images.o
${COMPILER}/${TARGET_FILE}: ${COMPILER}/Powertip320x480x16_st7796s_spi.o
${COMPILER}/${TARGET_FILE}: ${COMPILER}/startup_${COMPILER}.o
ifeq (${SRAM}, 1)
${COMPILER}/${TARGET_FILE}: ${COMPILER}/sram_startup_${COMPILER}.o
endif
${COMPILER}/${TARGET_FILE}: ${COMPILER}/fl380-v2.o
# ${COMPILER}/${TARGET_FILE}: ${COMPILER}/uartstdio.o
${COMPILER}/${TARGET_FILE}: ${COMPILER}/ustdlib.o
${COMPILER}/${TARGET_FILE}: ${ROOT}/grlib/${COMPILER}/libgr.a
${COMPILER}/${TARGET_FILE}: ${ROOT}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/${TARGET_FILE}: ${TARGET_NAME}.ld
ifeq (${SRAM}, 1)
SCATTERgcc_${TARGET_NAME}=${TARGET_NAME}_sram.ld
ENTRY_${TARGET_NAME}=SramExecInit
$(info Build to SRAM)
else
SCATTERgcc_${TARGET_NAME}=${TARGET_NAME}.ld
ENTRY_${TARGET_NAME}=ResetISR
$(info Build to FLASH)
endif
CFLAGSgcc=-DTARGET_IS_TM4C129_RA0 -DUART_BUFFERED
# CFLAGSgcc=-DUART_BUFFERED

#
# Include the automatically generated dependency files.
#
ifneq (${MAKECMDGOALS},clean)
-include ${wildcard ${COMPILER}/*.d} __dummy__
endif
