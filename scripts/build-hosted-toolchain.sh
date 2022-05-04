#!/bin/bash

# sudo apt install m4 flex bison
# brew install isl libmpc mpfr pcre2

CPU_CORES=1
if [[ $OSTYPE == 'darwin'* ]]; then
  CPU_CORES=$(sysctl -n hw.logicalcpu)
  export CFLAGS="$CFLAGS -I/usr/local/include"
  export LDFLAGS="$LDFLAGS -L/usr/local/lib"
elif [[ $OSTYPE == 'linux'* ]]; then
  CPU_CORES=$(nproc --all)
fi


echo "Detected $CPU_CORES CPU core(s)"
sleep 1

REPO=$(pwd)
SYSROOT=$REPO/alloy
PREFIX="$HOME/opt/cross"

AUTOCONF_VERSION="2.69"
AUTOMAKE_VERSION="1.15.1"
BINUTILS_VERSION="2.38"
GCC_VERSION="11.2.0"

cd build || (echo "missing build dir"; exit)

TOOLS_PREFIX="$REPO/build/binutils-tools"
export PATH="$TOOLS_PREFIX/bin:$PATH"

if [[ ! -f $TOOLS_PREFIX/bin/autoconf ]]; then
  mkdir -p $TOOLS_PREFIX

  if [[ ! -d autoconf-$AUTOCONF_VERSION ]]; then
    if [[ ! -f "autoconf-$AUTOCONF_VERSION.tar.xz" ]]; then
      curl https://ftp.gnu.org/gnu/autoconf/autoconf-$AUTOCONF_VERSION.tar.xz -o "autoconf-$AUTOCONF_VERSION.tar.xz"
    fi

    tar xf autoconf-$AUTOCONF_VERSION.tar.xz
  fi

  rm -rf autoconf
  mkdir autoconf
  cd autoconf || exit
  ../autoconf-$AUTOCONF_VERSION/configure --prefix="$TOOLS_PREFIX"
  make -j$CPU_CORES
  make install
  cd $REPO/build || exit
fi

if [[ ! -f $TOOLS_PREFIX/bin/automake ]]; then
  cd $REPO/build || exit

  if [[ ! -d automake-$AUTOMAKE_VERSION ]]; then
    if [[ ! -f "automake-$AUTOMAKE_VERSION.tar.xz" ]]; then
      curl "https://ftp.gnu.org/gnu/automake/automake-$AUTOMAKE_VERSION.tar.xz" -o "automake-$AUTOMAKE_VERSION.tar.xz"
    fi

    tar xf automake-$AUTOMAKE_VERSION.tar.xz || exit
  fi

  echo $TOOLS_PREFIX

  rm -rf automake
  mkdir automake
  cd automake || exit
  ../$AUTOMAKE_VERSION/configure --prefix="$TOOLS_PREFIX"
  make -j$CPU_CORES
  make install
  cd $REPO/build || exit
fi

SYSROOT=$REPO/alloy

if [[ ! -f $SYSROOT/usr/local/bin/i686-alloy-as ]]; then
  cd $REPO/build || exit

  if [[ ! -d $REPO/build/binutils-alloy ]]; then
    if [[ ! -d "$REPO/build/binutils-$BINUTILS_VERSION" ]]; then
      if [[ ! -f  "$REPO/build/binutils-$BINUTILS_VERSION.tar.xz" ]]; then
        curl "https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz" -o "binutils-$BINUTILS_VERSION.tar.xz" || exit
      fi

      tar xf "binutils-$BINUTILS_VERSION.tar.xz"
    fi

    cp -r "$REPO/build/binutils-$BINUTILS_VERSION" $REPO/build/binutils-alloy

    cd $REPO/toolchain/binutils/patches || exit
    find . -type f -exec patch -u $REPO/build/binutils-alloy/{} -i {} \;

    cd $REPO/toolchain/binutils/files || exit
    cp -r ./* $REPO/build/binutils-alloy/

    cd $REPO/build/binutils-alloy/ld || exit
    automake
  fi

  cd $REPO/build || exit
  rm -rf build-binutils-alloy
  mkdir build-binutils-alloy
  cd build-binutils-alloy || exit
  ../binutils-alloy/configure --with-sysroot="$SYSROOT" --target=i686-alloy --disable-werror
  
  make -j$CPU_CORES
  make DESTDIR="${SYSROOT}" install

  if [[ ! -f $SYSROOT/usr/local/bin/i686-alloy-as ]]; then
    echo "Couldn't find SYSROOT/usr/local/bin/i686-alloy-as"
    echo "There must have been an error buiding binutils. Exiting"
    exit
  fi
fi


if [[ ! -f $SYSROOT/usr/local/bin/i686-alloy-gcc ]]; then
  cd $REPO/build

  if [[ ! -d "gcc-alloy" ]]; then
    if [[ ! -d gcc-$GCC_VERSION ]]; then
      if [[ ! -f "gcc-$GCC_VERSION.tar.xz" ]]; then
        curl "http://gnu.mirrors.hoobly.com/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz" -o "gcc-$GCC_VERSION.tar.xz" || exit
      fi

      tar xf "gcc-$GCC_VERSION.tar.xz" || exit
    fi

    cp -r "gcc-$GCC_VERSION" gcc-alloy

    cd $REPO/toolchain/gcc/files
    cp -r ./* $REPO/build/gcc-alloy

    cd $REPO/toolchain/gcc/patches
    find . -type f -exec patch -u $REPO/build/gcc-alloy/{} -i {} \;

    cd $REPO/build/gcc-alloy/libstdc++-v3
    autoconf
  fi

  cd $REPO/build
  rm -rf build-gcc-alloy
  mkdir build-gcc-alloy
  cd build-gcc-alloy

  ../gcc-alloy/configure --target=i686-alloy --with-sysroot="$SYSROOT" --disable-werror \
    --with-newlib --disable-shared --disable-threads --enable-languages=c
  make -j$CPU_CORES all-gcc
  make -j$CPU_CORES all-target-libgcc

  make DESTDIR="${SYSROOT}" install-gcc
  make DESTDIR="${SYSROOT}" install-target-libgcc

  if [[ ! -f $SYSROOT/usr/local/bin/i686-alloy-gcc ]]; then
    echo "*** WARNING: Couldn't find i686-alloy-gcc in SYSROOT - there may have been an issue"
  fi

  if [[ ! -f "$SYSROOT/usr/local/lib/gcc/i686-alloy/$GCC_VERSION/libgcc.a" ]]; then
    echo "*** WARNING: Couldn't find libgcc.a - there may have been an issue"
  fi
fi

cd $REPO
