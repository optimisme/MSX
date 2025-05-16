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
    echo "Usage:    $0 {compile|run} {background|bank_mapper|bitmap|hello|scroll|smooth_scroll_x|smooth_scroll_y} [phillips]"
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
        background)
            zcc +msx -DSCREEN_MODE=2 \
                    -create-app -subtype=rom -lmsxbios -lmsx_clib \
                    -O3 --opt-code-speed \
                src/$target/fps.c \
                src/$target/main.c \
                src/$target/sprites.c \
                src/$target/tiles.c \
                src/utils/utils_msx.asm \
                src/utils/utils_msx.c \
                -o "$out_dir/app"
            ;;
        bank_mapper)
            zcc +msx -create-app -subtype=rom -lmsxbios -lmsx_clib \
                     -pragma-define:MAPPER_ASCII16 \
                     -O3 --opt-code-speed \
                src/$target/main.c \
                src/$target/bank1.c \
                src/$target/bank2.c \
                -o "$out_dir/app"
            ;;
        bitmap)
            zcc +msx -DSCREEN_MODE=2 \
                     -create-app -subtype=rom -lmsxbios -lmsx_clib \
                     -O3 --opt-code-speed \
                src/$target/fps.c \
                src/$target/main.c \
                src/$target/sprites.c \
                src/$target/tiles.c \
                src/utils/utils_msx.asm \
                src/utils/utils_msx.c \
                -o "$out_dir/app"
            ;;
        hello)
            zcc +msx -create-app -subtype=rom -lmsxbios \
                "src/$target/main.c" \
                -o "$out_dir/app"
            ;;
        scroll|smooth_scroll_x|smooth_scroll_y)
            zcc +msx -DSCREEN_MODE=2 \
                     -DAMALLOC \
                     -create-app -subtype=rom -lmsxbios -lmsx_clib \
                     -O3 --opt-code-speed \
                src/$target/fps.c \
                src/$target/loading.c \
                src/$target/main.c \
                src/$target/sprites.c \
                src/$target/tiles.c \
                src/utils/utils_msx.asm \
                src/utils/utils_msx.c \
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