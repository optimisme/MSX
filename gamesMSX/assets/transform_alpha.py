#!/usr/bin/env python3

import sys
import os
import re

try:
    from PIL import Image
except ImportError:
    print("Install Pillow: brew install Pillow")
    sys.exit(1)


def sanitize_identifier(name):
    # Substitueix tot el que no sigui alfanumèric per underscore
    s = re.sub(r'\W', '_', name)
    # Si comença amb dígit, prefixar underscore
    if re.match(r'^[0-9]', s):
        s = '_' + s
    return s


def main():
    if len(sys.argv) != 2:
        print("Usage: python transform_apha.py input.png")
        return
    input_path = sys.argv[1]
    base = os.path.splitext(os.path.basename(input_path))[0]
    identifier = sanitize_identifier(base)
    guard = identifier.upper() + "_H"

    # Carrega la imatge
    try:
        img = Image.open(input_path)
    except Exception as e:
        print(f"Error opening image: {e}")
        return

    # Convertim a escala de grisos
    img = img.convert('L')
    width, height = img.size

    # Dimensions en nombre de tiles
    if width % 8 != 0 or height % 8 != 0:
        print("Image with and height must be multiple of 8 píxels.")
        return
    cols = width // 8
    rows = height // 8
    num_tiles = rows * cols

    # Generació del .h
    h_lines = []
    h_lines.append(f"#ifndef {guard}")
    h_lines.append(f"#define {guard}")
    h_lines.append("")
    h_lines.append(f"#define NUM_TILES {num_tiles}")
    h_lines.append("#define TILE_BYTES 8")
    h_lines.append("")
    h_lines.append(f"extern const unsigned char {identifier}_bitmap[NUM_TILES][TILE_BYTES];")
    h_lines.append("")
    h_lines.append(f"#endif /* {guard} */")

    h_filename = f"{base}.h"
    with open(h_filename, "w") as f:
        f.write("\n".join(h_lines))
    print(f"{h_filename} generated.")

    # Generació del .c
    c_lines = []
    c_lines.append(f'#include "{h_filename}"')
    c_lines.append("")
    c_lines.append(f"const unsigned char {identifier}_bitmap[NUM_TILES][TILE_BYTES] = {{")

    pixels = img.load()
    # Recorrer per tessel·les d'esquerra a dreta, dalt a baix
    for tile_index in range(num_tiles):
        r = tile_index // cols
        c = tile_index % cols
        # Inici del tile
        byte_vals = []
        for y in range(8):
            byte = 0
            for x in range(8):
                px = pixels[c*8 + x, r*8 + y]
                bit = 1 if px > 127 else 0
                byte = (byte << 1) | bit
            byte_vals.append(f"0x{byte:02x}")
        line = "    { " + ", ".join(byte_vals) + " },"
        c_lines.append(line)

    c_lines.append("};")

    c_filename = f"{base}.c"
    with open(c_filename, "w") as f:
        f.write("\n".join(c_lines))
    print(f"{c_filename} generated.")


if __name__ == '__main__':
    main()
