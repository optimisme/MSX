#!/usr/bin/env python3
"""
MSX sprite transformer:
- Splits una imatge en sprites 16×16 (4 blocs de 8×8)
- Deduplica els sprites per construir un spriteset
- Genera un spritemap
- Exporta .h/.c amb:
    ident_bitmap_spriteset   [N][32]
    ident_bitmap_sprites     [rows*cols]
"""
import sys, os, re, argparse
from PIL import Image

BIT_THRESHOLD = 128

def sanitize_identifier(name):
    s = re.sub(r'\W', '_', name)
    if re.match(r'^[0-9]', s):
        s = '_' + s
    return s

def generate_header(base, ident, rows, cols, sprite_bytes, sprite_count, out_h, pragma_bank=None):
    guard = f"{ident.upper()}_H"
    lines = []
    if pragma_bank is not None:
        lines.append(f"#pragma bank {pragma_bank}")
    lines += [
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        "#include <stdint.h>",
        "",
        f"#define {ident.upper()}_SPRITE_ROWS    {rows}",
        f"#define {ident.upper()}_SPRITE_COLUMNS {cols}",
        f"#define {ident.upper()}_SPRITE_BYTES   {sprite_bytes}",
        f"#define {ident.upper()}_SPRITE_COUNT   {sprite_count}",
        "",
        f"extern const uint8_t {ident}_bitmap_spriteset"
        f"[{ident.upper()}_SPRITE_COUNT]"
        f"[{ident.upper()}_SPRITE_BYTES];",
        f"extern const uint8_t {ident}_bitmap_sprites"
        f"[{ident.upper()}_SPRITE_ROWS * {ident.upper()}_SPRITE_COLUMNS];",
        "",
        f"#endif /* {guard} */"
    ]
    with open(out_h, 'w') as f:
        f.write("\n".join(lines))

def generate_source(base, ident, spriteset, spritemap, out_c, header_file):
    lines = [f'#include "{header_file}"', ""]
    # spriteset
    lines.append(
        f"const uint8_t {ident}_bitmap_spriteset"
        f"[{ident.upper()}_SPRITE_COUNT]"
        f"[{ident.upper()}_SPRITE_BYTES] = {{"
    )
    for tile in spriteset:
        hexs = ", ".join(f"0x{b:02X}" for b in tile)
        lines.append(f"    {{ {hexs} }},")
    lines.append("};\n")
    # spritemap
    lines.append(
        f"const uint8_t {ident}_bitmap_sprites"
        f"[{ident.upper()}_SPRITE_ROWS * {ident.upper()}_SPRITE_COLUMNS] = {{"
    )
    flat = [str(v) for row in spritemap for v in row]
    # agrupar per files de 16 si vols llegible, però 1D és OK
    lines.append("    " + ", ".join(flat) + ",")
    lines.append("};")
    with open(out_c, 'w') as f:
        f.write("\n".join(lines))

def main():
    p = argparse.ArgumentParser(
        description="Converteix una imatge de sprites 16×16 en .h/.c per MSX"
    )
    p.add_argument('-p','--pragma', type=int,
                   help='Opcional: pragmabank per al header')
    p.add_argument('input_image',
                   help='Imatge d’entrada (amplada i alçada múltiples de 16)')
    args = p.parse_args()

    base = os.path.splitext(os.path.basename(args.input_image))[0]
    ident = sanitize_identifier(base)
    out_h = f"{base}.h"
    out_c = f"{base}.c"

    try:
        img = Image.open(args.input_image).convert('L')
    except Exception as e:
        print(f"Error obrint la imatge: {e}")
        sys.exit(1)

    w, h = img.size
    if w % 16 or h % 16:
        print("Les dimensions han de ser múltiples de 16.")
        sys.exit(1)

    pixels = img.load()
    cols = w // 16
    rows = h // 16

    sprite_dict = {}
    spriteset    = []
    spritemap    = [[0]*cols for _ in range(rows)]

    for ry in range(rows):
        for cx in range(cols):
            left_bytes  = []
            right_bytes = []
            # 1. recollim 16 bytes de l'esquerra i 16 de la dreta
            for y in range(16):
                b0 = 0
                for x in range(8):
                    bit = 1 if pixels[cx*16 + x, ry*16 + y] > BIT_THRESHOLD else 0
                    b0 = (b0 << 1) | bit
                left_bytes.append(b0)

                b1 = 0
                for x in range(8, 16):
                    bit = 1 if pixels[cx*16 + x, ry*16 + y] > BIT_THRESHOLD else 0
                    b1 = (b1 << 1) | bit
                right_bytes.append(b1)

            # 2. concat TL, TR, BL, BR
            data = (
                left_bytes[0:8]   +  # top-left
                left_bytes[8:16]  +  # bottom-left
                right_bytes[0:8]  +  # top-right
                right_bytes[8:16]   
            )

            key = tuple(data)
            idx = sprite_dict.setdefault(key, len(spriteset))
            if idx == len(spriteset):
                spriteset.append(key)
            spritemap[ry][cx] = idx

    sprite_bytes = 32
    sprite_count = len(spriteset)

    generate_header(base, ident, rows, cols,
                    sprite_bytes, sprite_count, out_h, args.pragma)
    generate_source(base, ident, spriteset, spritemap, out_c, out_h)

    print(f"Generats {sprite_count} sprites únics ({sprite_bytes} bytes cadascun).")
    print(f"Fitxers creats: {out_h}, {out_c}")

if __name__ == "__main__":
    main()
