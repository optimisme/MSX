#!/bin/bash
set -e
set -o pipefail

# ./make-msx.sh compile scroll [phillips]
# ./make-msx.sh run scroll [phillips]

if [ -t 1 ]; then
    reset
fi

export ZCCCFG="$(pwd)/../z88dk-msx/lib/config"
export PATH="$(pwd)/../z88dk-msx/bin:$PATH"

# Paràmetres
action="${1:-}"
target="${2:-}"
machine_param="${3:-}"

# Comprovació d'ús
if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    echo "Usage:    $0 {compile|run} {minigames} [phillips]"
    echo "Examples: $0 compile minigames"
    echo "          $0 run minigames phillips"
    exit 1
fi

if [ "$action" != "compile" ] && [ "$action" != "run" ]; then
    echo "Unknown action: $action"
    exit 1
fi

# Dependency checks
PYTHON_BIN="${PYTHON_BIN:-python3}"
if ! command -v "$PYTHON_BIN" >/dev/null 2>&1; then
    echo "Missing python3. Install it and try again."
    exit 1
fi
if ! "$PYTHON_BIN" - <<'PY' >/dev/null 2>&1; then
from PIL import Image
PY
    echo "Missing Pillow (PIL). Install with: $PYTHON_BIN -m pip install pillow"
    exit 1
fi
if [ ! -d "$ZCCCFG" ]; then
    echo "Missing Z88DK config at $ZCCCFG"
    echo "Run ./install-z88dk.sh from the repo root to install z88dk."
    exit 1
fi
if ! command -v zcc >/dev/null 2>&1; then
    echo "Missing z88dk compiler (zcc). Run ./install-z88dk.sh to install it."
    exit 1
fi

# Configuració de la màquina per a run
machine_flag=""
if [ -n "$machine_param" ]; then
    if [ "$machine_param" = "phillips" ]; then
        machine_flag="-machine Philips_VG8020"
    else
        echo "Unknown option: $machine_param"
        exit 1
    fi
fi

out_dir="build"
mkdir -p "$out_dir"

compile() {
    case "$target" in
        minigames)
            cd assets
            "$PYTHON_BIN" transform_bitmap.py alphabet_bitmap.png
            rm -f ../src/minigames/alphabet_bitmap.*
            mv -f alphabet_bitmap.h ../src/minigames/alphabet/
            mv -f alphabet_bitmap.c ../src/minigames/alphabet/
            "$PYTHON_BIN" transform_bitmap.py menu_bitmap.png --pragma 1
            rm -f ../src/minigames/menu/menu_bitmap.*
            mv -f menu_bitmap.h ../src/minigames/menu/
            mv -f menu_bitmap.c ../src/minigames/menu/
            "$PYTHON_BIN" transform_bitmap.py g_2048_bitmap.png --pragma 2
            rm -f ../src/minigames/menu/g_2048_bitmap.*
            mv -f g_2048_bitmap.h ../src/minigames/g_2048/
            mv -f g_2048_bitmap.c ../src/minigames/g_2048/
            "$PYTHON_BIN" transform_sprites16.py g_2048_sprites.png --pragma 2
            rm -f ../src/minigames/menu/g_2048_sprites.*
            mv -f g_2048_sprites.h ../src/minigames/g_2048/
            mv -f g_2048_sprites.c ../src/minigames/g_2048/
            # python transform_alpha.py alpha.png
            # mv -f alpha.h ../src/utils/
            # mv -f alpha.c ../src/utils/
            cd ..
            zcc +msx \
                    -create-app -subtype=rom -lmsxbios \
                    -pragma-define:MAPPER_ASCII16 \
                    -O3 --opt-code-speed \
                src/$target/*.c \
                src/$target/*/*.c \
                src/utils/*.c \
                -o "$out_dir/app"
            ;;
        *)
            echo "Unknown target: $target"
            exit 1
            ;;
    esac
}

# Compilar
compile
compile_result=$?

if [ $compile_result -ne 0 ]; then
    echo "❌ Compilation failed. Emulator will not be started."
    exit 1
fi

# Executar
if [ "$action" = "run" ]; then
    if ! command -v openmsx >/dev/null 2>&1; then
        echo "Missing openMSX emulator (openmsx). Run ./install-openmsx.sh to install it."
        exit 1
    fi
    openmsx $machine_flag -cart "$out_dir/app.rom"
fi
