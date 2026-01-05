#!/bin/bash
set -e
set -o pipefail

brew install gmp mtools
brew install cmake make gcc

if [ ! -d "z88dk-msx/.git" ]; then
  if [ -d "z88dk-msx" ]; then
    echo "z88dk-msx exists but is not a git repo; remove it or fix it before running this installer."
    exit 1
  fi
  git clone https://github.com/z88dk/z88dk.git z88dk-msx
fi

cd z88dk-msx
git submodule update --init --recursive
export Z88DK=$(pwd)
export PATH=$Z88DK/bin:$PATH
export ZCCCFG=$Z88DK/lib/config
export CPPFLAGS="-I/opt/homebrew/include"
export LDFLAGS="-L/opt/homebrew/lib -lgmp"
./build.sh -p msx
