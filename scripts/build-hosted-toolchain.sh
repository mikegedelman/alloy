#!/bin/bash

# sudo apt install m4 flex bison

REPO=$(pwd)
SYSROOT=$REPO/alloy
PREFIX="$HOME/opt/cross"

AUTOCONF_VERSION="autoconf-2.69"
AUTOMAKE_VERSION="automake-1.15.1"
BINUTILS_VERSION="binutils-2.38"
GCC_VERSION="gcc-11.2.0"

cd build || (echo "missing build dir"; exit)

TOOLS_PREFIX="$REPO/build/binutils-tools"
export PATH="$TOOLS_PREFIX/bin:$PATH"

if [[ ! -f $TOOLS_PREFIX/bin/autoconf ]]; then
  mkdir -p $TOOLS_PREFIX

  if [[ ! -d $AUTOCONF_VERSION ]]; then
    if [[ ! -f "$AUTOCONF_VERSION.tar.xz" ]]; then
      curl https://ftp.gnu.org/gnu/autoconf/$AUTOCONF_VERSION.tar.xz -o "$AUTOCONF_VERSION.tar.xz"
    fi

    tar xf $AUTOCONF_VERSION.tar.xz
  fi

  rm -rf autoconf
  mkdir autoconf
  cd autoconf || exit
  ../$AUTOCONF_VERSION/configure --prefix="$TOOLS_PREFIX"
  make -j15
  make install
  cd $REPO/build || exit
fi

if [[ ! -f $TOOLS_PREFIX/bin/automake ]]; then
  cd $REPO/build || exit

  if [[ ! -d $AUTOMAKE_VERSION ]]; then
    if [[ ! -f "$AUTOMAKE_VERSION.tar.xz" ]]; then
      curl "https://ftp.gnu.org/gnu/automake/$AUTOMAKE_VERSION.tar.xz" -o "$AUTOMAKE_VERSION.tar.xz"
    fi

    tar xf $AUTOMAKE_VERSION.tar.xz || exit
  fi

  echo $TOOLS_PREFIX

  rm -rf automake
  mkdir automake
  cd automake || exit
  ../$AUTOMAKE_VERSION/configure --prefix="$TOOLS_PREFIX"
  make -j15
  make install
  cd $REPO/build || exit
fi

SYSROOT=$REPO/alloy

if [[ ! -f $SYSROOT/usr/local/bin/i686-alloy-as ]]; then
  cd $REPO/build || exit

  if [[ ! -d $REPO/build/binutils-alloy ]]; then
    if [[ ! -d "$REPO/build/$BINUTILS_VERSION" ]]; then
      if [[ ! -f  "$REPO/build/$BINUTILS_VERSION.tar.xz" ]]; then
        curl "https://ftp.gnu.org/gnu/binutils/$BINUTILS_VERSION.tar.xz" -o "$BINUTILS_VERSION.tar.xz" || exit
      fi

      tar xf "$BINUTILS_VERSION.tar.xz"
    fi

    cp -r "$REPO/build/$BINUTILS_VERSION" $REPO/build/binutils-alloy

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
  
  make -j15
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
    if [[ ! -d $GCC_VERSION ]]; then
      if [[ ! -f "$GCC_VERSION.tar.xz" ]]; then
        curl "http://gnu.mirrors.hoobly.com/gcc/$GCC_VERSION/$GCC_VERSION.tar.xz" -o "$GCC_VERSION.tar.xz" || exit
      fi

      tar xf "$GCC_VERSION.tar.xz" || exit
    fi

    cp -r "$GCC_VERSION" gcc-alloy

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
  make -j15 all-gcc
  make -j15 all-target-libgcc

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
