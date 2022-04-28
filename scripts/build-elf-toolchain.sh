#!/bin/bash

# sudo apt install build-essential texinfo libgmp-dev libmpfr-dev libmpc-dev

CPU_CORES=1
if [[ $OSTYPE == 'darwin'* ]]; then
  CPU_CORES=$(sysctl -n hw.logicalcpu)
elif [[ $OSTYPE == 'linux'* ]]; then
  CPU_CORES=$(nproc --all)
fi

echo "Detected $CPU_CORES CPU core(s)"
sleep 1

PREFIX="$HOME/opt/cross"
TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

mkdir -p $PREFIX
mkdir -p build
cd build || exit

if [[ ! -f  binutils-2.38.tar.xz ]]; then
  curl https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.xz -o binutils-2.38.tar.xz
fi

tar xf binutils-2.38.tar.xz
rm -rf binutils
mkdir binutils
cd binutils || exit
../binutils-2.38/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$CPU_CORES
make install

if [[ ! -f gcc-11.2.0.tar.xz ]]; then
  curl http://gnu.mirrors.hoobly.com/gcc/gcc-11.2.0/gcc-11.2.0.tar.xz -o gcc-11.2.0.tar.xz
fi

# Build GCC
which -- $TARGET-as || echo $TARGET-as is not in the PATH
tar xf gcc-11.2.0.tar.xz
rm -rf gcc
mkdir gcc
cd gcc || exit
# make distclean
../gcc-11.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make -j$CPU_CORES all-gcc
make -j$CPU_CORES all-target-libgcc
make install-gcc
make install-target-gcc
cd ..

cd ..