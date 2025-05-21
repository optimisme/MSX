#!/usr/bin/env python3

import sys
import os
import re

try:
    from PIL import Image
except ImportError:
    print("Cal instal·lar Pillow: pip install Pillow")
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
        print("Usage: python transform.py input.png")
        return
    input_path = sys.argv[1]
    base = os.path.splitext(os.path.basename(input_path))[0]
    identifier = sanitize_identifier(base)
    identifier_upper = identifier.upper()
    guard = identifier_upper + "_H"

    try:
        img = Image.open(input_path)
    except Exception as e:
        print(f"Error opening image: {e}")
        return

    img = img.convert('L')
    width, height = img.size

    pixels = img.load()
    rows = height // 8
    cols = width // 8

    # Generació del .h
    header = []
    header.append(f"#ifndef {guard}")
    header.append(f"#define {guard}")
    header.append("")
    header.append(f"#define {identifier_upper}_MAP_ROWS {rows}")
    header.append(f"#define {identifier_upper}_MAP_COLUMNS {cols}")
    header.append(f"#define {identifier_upper}_TILE_BYTES 8")
    header.append("")
    header.append(f"extern const unsigned char {identifier}[{identifier_upper}_MAP_ROWS][{identifier_upper}_MAP_COLUMNS][{identifier_upper}_TILE_BYTES];")
    header.append("")
    header.append(f"#endif /* {guard} */")

    h_filename = f"{base}.h"
    with open(h_filename, "w") as f:
        f.write("\n".join(header))
    print(f"{h_filename} generated.")

    # Generació del .c
    source = []
    source.append(f'#include "{h_filename}"')
    source.append("")
    source.append(f"const unsigned char {identifier}[{identifier_upper}_MAP_ROWS][{identifier_upper}_MAP_COLUMNS][{identifier_upper}_TILE_BYTES] = {{")
    for r in range(rows):
        source.append("    {")
        for c in range(cols):
            bytes_hex = []
            for y in range(8):
                byte = 0
                for x in range(8):
                    px = pixels[c*8 + x, r*8 + y]
                    bit = 1 if px > 127 else 0
                    byte = (byte << 1) | bit
                bytes_hex.append(f"0x{byte:02x}")
            byte_str = ", ".join(bytes_hex)
            source.append(f"        {{ {byte_str} }},")
        source.append("    },")
    source.append("};")

    c_filename = f"{base}.c"
    with open(c_filename, "w") as f:
        f.write("\n".join(source))
    print(f"{c_filename} generated.")

if __name__ == '__main__':
    main()

