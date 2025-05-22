#!/bin/bash

# ./make-msx.sh compile scroll [phillips]
# ./make-msx.sh run scroll [phillips]

export ZCCCFG="$(pwd)/../z88dk-msx/lib/config"
export PATH="$(pwd)/../z88dk-msx/bin:$PATH"

# Paràmetres
action="$1"
target="$2"
machine_param="$3"

# Comprovació d'ús
if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    echo "Usage:    $0 {compile|run} {minigames} [phillips]"
    echo "Examples: $0 compile scroll"
    echo "          $0 run scroll phillips"
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
            python transform_bitmap.py menu_bitmap.png --pragma 1
            rm -f ../src/minigames/menu/menu_bitmap.*
            cp -f menu_bitmap.h ../src/minigames/menu/
            cp -f menu_bitmap.c ../src/minigames/menu/
            python transform_bitmap.py g2048_bitmap.png --pragma 2
            rm -f ../src/minigames/menu/g2048_bitmap.*
            cp -f g2048_bitmap.h ../src/minigames/g2048/
            cp -f g2048_bitmap.c ../src/minigames/g2048/
            python transform_sprites16.py g2048_sprites.png --pragma 2
            rm -f ../src/minigames/menu/g2048_sprites.*
            cp -f g2048_sprites.h ../src/minigames/g2048/
            cp -f g2048_sprites.c ../src/minigames/g2048/
            # python transform_alpha.py alpha.png
            # cp -f alpha.h ../src/utils/
            # cp -f alpha.c ../src/utils/
            cd ..
            zcc +msx \
                    -create-app -subtype=rom -lmsxbios \
                    -pragma-define:MAPPER_ASCII16 \
                    -O3 --opt-code-speed \
                src/$target/*.c \
                src/$target/*/*.c \
                src/utils/*.asm \
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
    openmsx $machine_flag -cart "$out_dir/app.rom"
fi