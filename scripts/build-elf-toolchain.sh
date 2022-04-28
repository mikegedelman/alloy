#!/bin/bash

# sudo apt install build-essential texinfo libgmp-dev libmpfr-dev libmpc-dev

REPO=$(pwd)

BINUTILS_VERSION="2.38"
GCC_VERSION="11.2.0"

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

if [[ ! -f  binutils-$BINUTILS_VERSION.tar.xz ]]; then
  curl https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz -o binutils-$BINUTILS_VERSION.tar.xz
fi

tar xf binutils-$BINUTILS_VERSION.tar.xz
rm -rf binutils
mkdir binutils
cd binutils || exit
../binutils-$BINUTILS_VERSION/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make -j$CPU_CORES
make install

if [[ ! -f $PREFIX/bin/i686-elf-as ]]; then
  echo "Couldn't find i686-elf-as at $PREFIX - something may have gone wrong. Aborting."
  exit
fi

if [[ ! -f "gcc-$GCC_VERSION.tar.xz" ]]; then
  curl "http://gnu.mirrors.hoobly.com/gcc/gcc-11.2.0/gcc-$GCC_VERSION.tar.xz" -o "gcc-$GCC_VERSION.tar.xz"
fi

# Build GCC
which -- $TARGET-as || echo $TARGET-as is not in the PATH
tar xf gcc-$GCC_VERSION.tar.xz
rm -rf gcc
mkdir gcc
cd gcc || exit
# make distclean
../gcc-$GCC_VERSION/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
make -j$CPU_CORES all-gcc
make -j$CPU_CORES all-target-libgcc
make install-gcc
make install-target-libgcc

if [[ ! -f $PREFIX/bin/i686-elf-gcc ]]; then
  echo "Couldn't find i686-elf-gcc at $PREFIX - something may have gone wrong."
fi

echo "i686-elf toolchain build complete. Installed at: $PREFIX"
echo "You may want to add $PREFIX/bin to your PATH."

cd $REPO

