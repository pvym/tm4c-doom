#!/usr/bin/env python3
"""Vygeneruje src/loading_screen_data.c (480x320, paleta + RLE) pro úvodní obrazovku.

Použití: pip install pillow && python3 tools/gen_loading_screen.py
Uprostřed je okno 320x200 (80,60), které potom přepíše scéna Dooma.
"""
import os
import random

from PIL import Image, ImageDraw, ImageFont

W, H = 480, 320
PX, PY, PW, PH = 80, 60, 320, 200
COLORS = 32
FONT_DIR = "/usr/share/fonts/truetype/liberation/"


def font(name, size):
    return ImageFont.truetype(FONT_DIR + name, size)


def centered(d, y, text, fnt, fill, shadow=None):
    w = d.textlength(text, font=fnt)
    x = PX + (PW - w) / 2
    if shadow:
        d.text((x + 2, y + 2), text, font=fnt, fill=shadow)
    d.text((x, y), text, font=fnt, fill=fill)


def build():
    random.seed(7)
    img = Image.new("RGB", (W, H), (20, 0, 0))
    d = ImageDraw.Draw(img)

    # Cihlová zeď kolem okna.
    for row in range(0, H, 16):
        off = 0 if (row // 16) % 2 == 0 else 20
        for col in range(-off, W, 40):
            r = random.randint(70, 110)
            d.rectangle([col + 1, row + 1, col + 38, row + 14], fill=(r, r // 5, r // 8))
            d.line([col + 1, row + 1, col + 38, row + 1], fill=(r + 30, r // 4 + 8, r // 6))
    # Lávové pruhy nahoře a dole.
    for y in range(24):
        t = y / 23.0
        top = (255, int(60 + 150 * t), 0)
        d.line([0, y, W, y], fill=top)
        d.line([0, H - 1 - y, W, H - 1 - y], fill=top)
    for x in range(0, W, 24):
        d.polygon([(x, 24), (x + 12, 40), (x + 24, 24)], fill=(255, 150, 0))
        d.polygon([(x, H - 25), (x + 12, H - 41), (x + 24, H - 25)], fill=(255, 150, 0))

    # Panel 320x200.
    d.rectangle([PX, PY, PX + PW - 1, PY + PH - 1], fill=(10, 10, 10))
    for i in range(3):
        d.rectangle([PX + i, PY + i, PX + PW - 1 - i, PY + PH - 1 - i],
                    outline=(200 - 40 * i, 20, 0))
    for y in range(PY + 3, PY + PH - 3):
        t = (y - PY) / PH
        d.line([PX + 3, y, PX + PW - 4, y], fill=(int(40 * (1 - t)) + 8, 0, 0))

    big = font("LiberationMono-Bold.ttf", 64)
    mid = font("LiberationMono-Bold.ttf", 20)
    small = font("LiberationMono-Bold.ttf", 16)

    centered(d, PY + 14, "DOOM", big, (235, 40, 0), shadow=(90, 0, 0))
    centered(d, PY + 86, "TM4C129 PORT", mid, (255, 200, 40))
    d.line([PX + 30, PY + 116, PX + PW - 31, PY + 116], fill=(160, 20, 0), width=2)
    centered(d, PY + 126, "pvym + GitHub Copilot", small, (210, 210, 210))
    centered(d, PY + 150, "doomgeneric / FatFs", small, (140, 140, 140))
    centered(d, PY + 172, "LOADING...", small, (255, 80, 0))
    return img


def main():
    img = build().quantize(colors=COLORS, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    pal = img.getpalette()[: COLORS * 3]
    pal565 = []
    for i in range(COLORS):
        r, g, b = pal[3 * i: 3 * i + 3]
        pal565.append(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

    get_pixels = getattr(img, "get_flattened_data", None) or img.getdata
    px = list(get_pixels())
    rle = []
    i = 0
    while i < len(px):
        v = px[i]
        n = 1
        while i + n < len(px) and px[i + n] == v and n < 255:
            n += 1
        rle += [n, v]
        i += n

    out = os.path.join(os.path.dirname(__file__), "..", "src", "loading_screen_data.c")
    with open(out, "w") as f:
        f.write("/* Generováno tools/gen_loading_screen.py - needitovat. */\n")
        f.write('#include "loading_screen.h"\n\n')
        f.write("const uint16_t g_loading_palette[LOADING_SCREEN_COLORS] = {\n")
        for k in range(0, COLORS, 8):
            f.write("    " + ", ".join("0x%04X" % c for c in pal565[k:k + 8]) + ",\n")
        f.write("};\n\n")
        f.write("const uint32_t g_loading_rle_len = %d;\n" % len(rle))
        f.write("const uint8_t g_loading_rle[] = {\n")
        for k in range(0, len(rle), 16):
            f.write("    " + ", ".join(str(c) for c in rle[k:k + 16]) + ",\n")
        f.write("};\n")
    print("RLE bytes:", len(rle))
    img.convert("RGB").save("/tmp/loading_preview.png")


if __name__ == "__main__":
    main()
