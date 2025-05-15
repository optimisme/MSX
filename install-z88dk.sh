brew install gmp mtools
brew install cmake make gcc
git clone https://github.com/z88dk/z88dk.git z88dk-msx
cd z88dk-msx
git submodule update --init --recursive
export Z88DK=$(pwd)
export PATH=$Z88DK/bin:$PATH
export ZCCCFG=$Z88DK/lib/config
export CPPFLAGS="-I/opt/homebrew/include"
export LDFLAGS="-L/opt/homebrew/lib -lgmp"
./build.sh -p msx

