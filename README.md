# tm4c-doom

Portace Doomgeneric na TI TM4C129XNCZAD pro bare-metal build s `arm-none-eabi-gcc`, TivaWare `SW-TM4C-2.2.0.295`, RGB565 LCD 480×320 na 16bit sběrnici a WAD soubor na microSD přes SSI1.

## Struktura

- `doomgeneric/` – vendored upstream zdrojáky Doomgeneric
- `fatfs/` – minimální FatFs vrstva pro read-only mount SD karty
- `inc/config.h` – adresy SDRAM/framebufferu, mapování GPIO vstupů, SSI1 piny
- `src/doomgeneric_tm4c.c` – SysTick timing, vstupní polling, paleta → RGB565, centrování 320×200 do 480×320
- `src/gpio_input.c` – inicializace a polling tlačítek
- `src/lcd_init.c` – framebuffer clear + weak hook pro board-specific LCD/EPI init
- `src/sd_driver.c` – SSI1 SPI-mode SD driver + FatFs disk I/O
- `src/w_file_fatfs.c` – Doom WAD backend nad FatFs s volitelným cache do SDRAM
- `linker/tm4c129_sdram.ld` – rozdělení SDRAM na zone heap, WAD cache a LCD framebuffer

## Výchozí mapování vstupů

Mapování je definováno v `inc/config.h`:

- `PA2` – šipka nahoru
- `PA3` – šipka dolů
- `PA4` – šipka vlevo
- `PA5` – šipka vpravo
- `PB4` – FIRE / shoot
- `PC4` – USE
- `PC5` – MENU / ESC

SD karta používá:

- `PB5` – SSI1 CLK
- `PE4` – SSI1 MOSI
- `PE5` – SSI1 MISO
- `PB0` – software CS (navržený výchozí pin)

Tlačítka jsou očekávána jako active-low se zapnutými interními pull-up rezistory.

## Paměťový layout

- `0x60000000 .. 0x60FFFFFF` – Doom zone heap (`16 MiB`)
- `0x61000000 .. 0x617FFFFF` – WAD cache (`8 MiB`)
- `0x10000000 .. 0x1004FFFF` – LCD framebuffer (`480 × 320 × RGB565`, `307200 B`)

Zbylá SDRAM není tímto linker skriptem využita.

## Build

Makefile očekává TivaWare instalované v `SW-TM4C-2.2.0.295` a startup soubor z TivaWare BSP. Pokud je máte jinde, předejte cesty při buildu:

```sh
make \
  TIVAWARE_DIR=/opt/ti/SW-TM4C-2.2.0.295 \
  STARTUP=/opt/ti/SW-TM4C-2.2.0.295/examples/boards/ek-tm4c1294xl/startup_gcc.c
```

Výstupy:

- `build/tm4c-doom.elf`
- `build/tm4c-doom.bin`
- `build/tm4c-doom.hex`

## Deployment

1. Připojte shareware nebo retail WAD na microSD jako `doom1.wad` v rootu karty.
2. Nahrajte `build/tm4c-doom.bin`/`.elf` do flash.
3. Doplňte ve vašem board-specific startupu nebo init kódu skutečnou inicializaci EPI/SDRAM/LCD a podle potřeby override-něte weak funkci `TM4C_LCD_ControllerInit()`.
4. Po startu port inicializuje clock, GPIO vstupy, SD kartu a spustí Doom s argumenty `-iwad 0:/doom1.wad -mb 16 -nosound`.

## Poznámky k implementaci

- Render zůstává bez škálování; `doomgeneric_tm4c.c` pouze centruje 320×200 obraz do 480×320 framebufferu a vyplní border černou.
- `src/w_file_fatfs.c` při dostatečné velikosti cache načte celý WAD do rezervované SDRAM oblasti, jinak čte přímo přes FatFs.
- Zvuk ani networking nejsou zapnuté.
