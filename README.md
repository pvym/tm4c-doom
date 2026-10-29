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

- `PE3` – šipka nahoru
- `PD4` – šipka dolů
- `PD5` – šipka vlevo
- `PD6` – šipka vpravo
- `PD7` – FIRE / shoot
- `PE7` – USE
- `PD1` – MENU / ESC

SD karta používá:

- `PB5` – SSI1 CLK
- `PB4` – SSI1 FSS / SD card CS (software-controlled)
- `PE4` – SSI1 MOSI
- `PE5` – SSI1 MISO

Tlačítka jsou očekávána jako active-low se zapnutými interními pull-up rezistory.

Při startu se na displeji zobrazí diagnostická barva aktuální fáze SD. Při
chybě program zůstane stát na barvě chyby:

- modrá – nastavení SPI sběrnice; červená znamená chybu této fáze
- žlutá – právě probíhá inicializace SD karty; při chybě:
  - tmavě červená – CMD0 (karta nepřešla do SPI idle režimu)
  - purpurová – CMD8 / kontrola R7
  - oranžová – ACMD41 (SD karta se neinicializovala)
  - tmavě fialová – CMD58 (čtení OCR)
  - žlutá – CMD16 (nastavení délky bloku pro starší kartu)
  - modrošedá – CMD1 (MMC karta se neinicializovala)
- azurová – připojení FAT filesystému; bílá znamená chybu filesystému
- zelená – všechny fáze uspěly a spouští se Doom

Úspěšné nastavení SPI ověřuje konfiguraci periferií a pinů; skutečnou
komunikaci s kartou ověřuje až inicializace karty.

Při chybě CMD0 se zobrazí osm světlých/tmavých polí zleva v pořadí bitů
`b7` až `b0` vrácené odpovědi R1; očekávaná odpověď je `0x01`. Spodní proužek
je modrý, pokud karta před příkazem neuvolnila MISO do stavu HIGH, jinak zelený.

## Paměťový layout

- `0x60000000 .. 0x60FFFFFF` – Doom zone heap (`16 MiB`)
- `0x61000000 .. 0x617FFFFF` – WAD cache (`8 MiB`)
- `0x10000000 .. 0x1004FFFF` – LCD framebuffer (`480 × 320 × RGB565`, `307200 B`)

Zbylá SDRAM není tímto linker skriptem využita.

## Build

Makefile nyní používá lokální `src/startup_gcc.c` odvozený z funkčního `x-v2` projektu. Externě je potřeba hlavně TivaWare `SW-TM4C-2.2.0.295`; pokud je jinde než v defaultu, předejte cestu při buildu:

```sh
make \
  TIVAWARE_DIR=/opt/ti/SW-TM4C-2.2.0.295
```

Výstupy:

- `build/tm4c-doom.elf`
- `build/tm4c-doom.bin`
- `build/tm4c-doom.hex`

## Deployment

1. Připojte shareware nebo retail WAD na microSD jako `doom1.wad` v rootu karty.
2. Nahrajte `build/tm4c-doom.bin`/`.elf` do flash.
3. `src/lcd_init.c` obsahuje raster/LCD pinmux a timing převzatý z `x-v2/display.c`; pokud potřebujete další board-specific kroky, doplňte je do `TM4C_LCD_ControllerInit()`.
4. Po startu port inicializuje clock, GPIO vstupy, SD kartu a spustí Doom s argumenty `-iwad 0:/doom1.wad -mb 16 -nosound`.

## Poznámky k implementaci

- Render zůstává bez škálování; `doomgeneric_tm4c.c` pouze centruje 320×200 obraz do 480×320 framebufferu a vyplní border černou.
- `src/w_file_fatfs.c` při dostatečné velikosti cache načte celý WAD do rezervované SDRAM oblasti, jinak čte přímo přes FatFs.
- Zvuk ani networking nejsou zapnuté.
- `src/lcd_init.c` nyní obsahuje i ST7796S RGB/SPI init sekvenci odvozenou z `x-v2/Powertip320x480x16_st7796s_spi.c`.
- Výchozí panel-control piny jsou v `inc/config.h` (`SSI0` na `PA2..PA5`, reset `PF1`, DRDX `PF0`, backlight `PH0`); pokud se vaše deska liší, upravte tyto makra.
