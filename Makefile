PROJECT := tm4c-doom
BUILD_DIR ?= build
TARGET := $(BUILD_DIR)/$(PROJECT)

CROSS_COMPILE ?= arm-none-eabi-
CC := $(CROSS_COMPILE)gcc
OBJCOPY := $(CROSS_COMPILE)objcopy
SIZE := $(CROSS_COMPILE)size

TIVAWARE_DIR ?= /opt/ti/SW-TM4C-2.2.0.295
STARTUP ?= src/startup_gcc.c
LINKER_SCRIPT ?= linker/tm4c129_sdram.ld

INCLUDES := -Iinc -Idoomgeneric -Ifatfs -I$(TIVAWARE_DIR)
DEFINES := \
    -DPART_TM4C129XNCZAD \
    -DTARGET_IS_TM4C129_RA2 \
    -DCMAP256 \
    -DDOOMGENERIC_RESX=320 \
    -DDOOMGENERIC_RESY=200 \
    -DTM4C_USE_FATFS \
    -DTM4C_FIXED_ZONE_HEAP \
    -DTM4C_DOOM_ZONE_HEAP_ADDR=0x60000000UL \
    -DTM4C_DOOM_ZONE_HEAP_SIZE=16777216UL
# CFLAGS := -std=c99 -Os -g3 -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
#     -ffunction-sections -fdata-sections -fno-common -Wall -Wextra $(INCLUDES) $(DEFINES)
#
#

CFLAGS := -std=c99 -Os -g3 -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard \
    -ffunction-sections -fdata-sections -fno-common -Wall -Wextra \
    -specs=nano.specs \
    $(INCLUDES) $(DEFINES)


LDFLAGS := -T$(LINKER_SCRIPT) -Wl,--gc-sections -Wl,-Map,$(TARGET).map
LIBS := $(TIVAWARE_DIR)/driverlib/gcc/libdriver.a -lm -lc

DOOM_SRCS := $(filter-out \
    doomgeneric/doomgeneric_allegro.c \
    doomgeneric/doomgeneric_emscripten.c \
    doomgeneric/doomgeneric_linuxvt.c \
    doomgeneric/doomgeneric_sdl.c \
    doomgeneric/doomgeneric_soso.c \
    doomgeneric/doomgeneric_sosox.c \
    doomgeneric/doomgeneric_win.c \
    doomgeneric/doomgeneric_xlib.c \
    doomgeneric/i_allegromusic.c \
    doomgeneric/i_allegrosound.c \
    doomgeneric/i_sdlmusic.c \
    doomgeneric/i_sdlsound.c, \
    $(wildcard doomgeneric/*.c))
PLATFORM_SRCS := \
    $(STARTUP) \
    src/main.c \
    src/doomgeneric_tm4c.c \
    src/gpio_input.c \
    src/lcd_init.c \
    src/sd_driver.c \
    src/syscalls.c \
    src/w_file_fatfs.c \
    fatfs/ff.c
SRCS := $(DOOM_SRCS) $(PLATFORM_SRCS)
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPFILES := $(OBJS:.o=.d)

.PHONY: all clean check-env print-config

all: check-env $(TARGET).elf $(TARGET).bin $(TARGET).hex

check-env:
	@command -v $(CC) >/dev/null 2>&1 || (echo "Missing compiler: $(CC)" && exit 1)
	@test -f "$(STARTUP)" || (echo "Missing startup file: $(STARTUP)" && exit 1)
	@test -f "$(TIVAWARE_DIR)/driverlib/gcc/libdriver.a" || (echo "Missing TivaWare driverlib under $(TIVAWARE_DIR)" && exit 1)

print-config:
	@echo "CC=$(CC)"
	@echo "TIVAWARE_DIR=$(TIVAWARE_DIR)"
	@echo "STARTUP=$(STARTUP)"
	@echo "LINKER_SCRIPT=$(LINKER_SCRIPT)"
	@printf '%s\n' $(SRCS)

$(TARGET).elf: $(OBJS) $(LINKER_SCRIPT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@ $(LIBS)
	$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

$(TARGET).hex: $(TARGET).elf
	$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPFILES)
