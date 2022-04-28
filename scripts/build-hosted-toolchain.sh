#!/bin/bash

# sudo apt install m4 flex bison

REPO=$(pwd)
SYSROOT=$REPO/alloy
PREFIX="$HOME/opt/cross"


cd build || (echo "missing build dir"; exit)

TOOLS_PREFIX="$REPO/build/binutils-tools"
export PATH="$TOOLS_PREFIX/bin:$PATH"

if [[ ! -f $TOOLS_PREFIX/bin/autoconf ]]; then
  mkdir -p $TOOLS_PREFIX

  if [[ ! -d autoconf-2.69 ]]; then
    if [[ ! -f autoconf-2.69.tar.xz ]]; then
      curl https://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.xz -o autoconf-2.69.tar.xz
    fi

    tar xf autoconf-2.69.tar.xz
  fi

  rm -rf autoconf
  mkdir autoconf
  cd autoconf || exit
  ../autoconf-2.69/configure --prefix="$TOOLS_PREFIX"
  make -j15
  make install
  cd $REPO/build || exit
fi

if [[ ! -f $TOOLS_PREFIX/bin/automake ]]; then
  if [[ ! -d automake-1.15.1 ]]; then
    if [[ ! -f automake-1.15.1.tar.xz ]]; then
      curl https://ftp.gnu.org/gnu/autoconf/automake-1.15.1.tar.xz -o automake-1.15.1.tar.xz
    fi

    tar xf automake-1.15.1.tar.xz
  fi

  echo $TOOLS_PREFIX

  rm -rf automake
  mkdir automake
  cd automake || exit
  ../automake-1.15.1/configure --prefix="$TOOLS_PREFIX"
  make -j15
  make install
  cd $REPO/build || exit
fi

SYSROOT=$REPO/alloy

if [[ ! -f $SYSROOT/usr/local/bin/i686-alloy-as ]]; then
  cd $REPO/build || exit

  cd $REPO/toolchain/binutils/patches || exit
  find . -type f -exec patch -u $REPO/build/binutils-alloy/{} -i {} \;

  cd $REPO/toolchain/bintuils/files || exit
  cp -r ./* $REPO/build/binutils-alloy/

  cd $REPO/build/binutils-alloy/ld || exit
  automake

  cd $REPO/build || exit
  rm -rf build-binutils-alloy
  mkdir build-binutils-alloy
  cd build-binutils-alloy || exit
  ../binutils-alloy/configure --with-sysroot="$SYSROOT" --target=i686-alloy --disable-werror
  
  make -j15
  make DESTDIR="${SYSROOT}" install
fi


if [[ ! -f $SYSROOT/usr/local/bin/gcc ]]; then
  cd $REPO/build
  # if [[ ! -d "gcc-11.2.0" ]]; then
  #   if [[ ! -f gcc-11.2.0.tar.xz ]]; then
  #     curl http://gnu.mirrors.hoobly.com/gcc/gcc-11.2.0/gcc-11.2.0.tar.xz -o gcc-11.2.0.tar.xz
  #   fi

  #   tar xf gcc-11.2.0.tar.xz
  # fi

  # cd $REPO/toolchain/gcc/files
  # cp -r ./* $REPO/build/gcc-alloy

  # cd $REPO/toolchain/gcc/patches
  # find . -type f -exec patch -u $REPO/build/gcc-alloy/{} -i {} \;

  # cd $REPO/build/gcc-alloy/libstdc++-v3
  # autoconf

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
fi

cd $REPO
