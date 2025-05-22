#!/usr/bin/env python3
"""
Advanced tile transformer:
- Splits an input image into 8x8 pixel tiles
- Deduplicates tiles to build a tileset
- Builds a tilemap referencing tileset indices
- Generates .h and .c files defining tileset and tilemap
"""
import sys
import os
import re
import argparse
from PIL import Image

BIT_THRESHOLD = 100

def sanitize_identifier(name):
    """
    Turn a file base name into a valid C identifier.
    Replace non-alphanumeric with underscore, prefix underscore if starting with digit.
    """
    s = re.sub(r'\W', '_', name)
    if re.match(r'^[0-9]', s):
        s = '_' + s
    return s


def generate_header(base, ident, rows, cols, tile_bytes, tile_count, out_h, pragma_bank=None):
    guard = f"{ident.upper()}_H"
    lines = []
    if pragma_bank is not None:
        lines.append(f"#pragma bank {pragma_bank}")
    lines.append(f"#ifndef {guard}")
    lines.append(f"#define {guard}")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("")
    lines.append(f"#define {ident.upper()}_MAP_ROWS {rows}")
    lines.append(f"#define {ident.upper()}_MAP_COLUMNS {cols}")
    lines.append(f"#define {ident.upper()}_TILE_BYTES {tile_bytes}")
    lines.append(f"#define {ident.upper()}_TILE_COUNT {tile_count}")
    lines.append("")
    lines.append(f"extern const uint8_t {ident}_tileset[{ident.upper()}_TILE_COUNT][{ident.upper()}_TILE_BYTES];")
    lines.append(f"extern const uint8_t {ident}_tilemap[{ident.upper()}_MAP_ROWS * {ident.upper()}_MAP_COLUMNS];")
    lines.append("")
    lines.append(f"#endif /* {guard} */")
    with open(out_h, 'w') as f:
        f.write("\n".join(lines))


def generate_source(base, ident, tileset, tilemap, out_c, header_file):
    rows = len(tilemap)
    cols = len(tilemap[0]) if rows > 0 else 0
    tile_bytes = len(tileset[0]) if tileset else 0

    lines = []
    lines.append(f'#include "{header_file}"')
    lines.append("")

    lines.append(
        f"const uint8_t {ident}_tileset"
        f"[{ident.upper()}_TILE_COUNT][{ident.upper()}_TILE_BYTES] = {{"
    )
    for key in tileset:
        hex_vals = ", ".join(f"0x{b:02x}" for b in key)
        lines.append(f"    {{ {hex_vals} }},")
    lines.append("};")
    lines.append("")

    lines.append(
        f"const uint8_t {ident}_tilemap"
        f"[{ident.upper()}_MAP_ROWS * {ident.upper()}_MAP_COLUMNS] = {{"
    )

    for r, row in enumerate(tilemap):
        vals = ",".join(f"{v:3d}" for v in row)
        lines.append(f"    {vals},")
    lines.append("};")

    with open(out_c, 'w') as f:
        f.write("\n".join(lines))


def main():
    parser = argparse.ArgumentParser(
        description="Split an image into tileset and tilemap with optional pragma bank"
    )
    parser.add_argument(
        '--pragma', '-p',
        type=int,
        help='Optional pragma bank number to add to the header file'
    )
    parser.add_argument(
        'input_image',
        help='Path to the input image (must be multiples of 8 in both dimensions)'
    )
    args = parser.parse_args()

    input_path = args.input_image
    base = os.path.splitext(os.path.basename(input_path))[0]
    ident = sanitize_identifier(base)
    out_h = f"{base}.h"
    out_c = f"{base}.c"

    try:
        img = Image.open(input_path)
    except Exception as e:
        print(f"Error opening image: {e}")
        sys.exit(1)

    img = img.convert('L')  # grayscale
    width, height = img.size
    if width % 8 != 0 or height % 8 != 0:
        print("Image dimensions must be multiples of 8.")
        sys.exit(1)

    cols = width // 8
    rows = height // 8
    pixels = img.load()

    tile_dict = {}  # maps tile data tuple -> index
    tileset = []    # list of unique tile data tuples
    tilemap = [[0] * cols for _ in range(rows)]

    for r in range(rows):
        for c in range(cols):
            data = []
            for y in range(8):
                byte = 0
                for x in range(8):
                    px = pixels[c*8 + x, r*8 + y]
                    bit = 1 if px > BIT_THRESHOLD else 0
                    byte = (byte << 1) | bit
                data.append(byte)
            key = tuple(data)
            if key not in tile_dict:
                tile_dict[key] = len(tileset)
                tileset.append(key)
            tilemap[r][c] = tile_dict[key]

    # Generate outputs with optional pragma
    generate_header(base, ident, rows, cols, 8, len(tileset), out_h, args.pragma)
    generate_source(base, ident, tileset, tilemap, out_c, out_h)

    print(f"Generated tileset with {len(tileset)} unique tiles.")
    print(f"Wrote {out_h} and {out_c}.")

if __name__ == '__main__':
    main()
