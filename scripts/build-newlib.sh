#!/bin/bash

REPO=$(pwd)
cd build || exit

TOOLS_PREFIX="$REPO/build/newlib-tools"
mkdir -p $TOOLS_PREFIX

export PATH="$TOOLS_PREFIX/bin:$PATH"

if [[ ! -f $TOOLS_PREFIX/bin/autoconf ]]; then

  curl https://ftp.gnu.org/gnu/autoconf/autoconf-2.65.tar.gz -o autoconf-2.65.tar.gz
  tar xf autoconf-2.65.tar.gz
  rm -rf autoconf
  mkdir autoconf
  cd autoconf || exit
  ../autoconf-2.65/configure --prefix="$TOOLS_PREFIX"
  make && make install
  cd $REPO/build || exit
else
  echo "Found autoconf: $TOOLS_PREFIX/bin/autoconf"
fi

if [[ ! -f $TOOLS_PREFIX/bin/automake ]]; then
  curl https://ftp.gnu.org/gnu/automake/automake-1.11.tar.gz -o automake-1.11.tar.gz
  tar xf automake-1.11.tar.gz
  rm -rf automake
  mkdir automake
  cd automake || exit
  ../automake-1.11/configure --prefix="$TOOLS_PREFIX"
  make && make install
  cd $REPO/build || exit
else
  echo "Found automake: $TOOLS_PREFIX/bin/automake"
fi

if [[ ! -d $REPO/build/newlib-alloy ]]; then
  if [[ ! -d newlib-2.5.0 ]]; then
    if [[ ! -f newlib-2.5.0.tar.gz ]]; then
      curl ftp://sourceware.org/pub/newlib/newlib-2.5.0.tar.gz -o newlib-2.5.0.tar.gz
    fi

    tar xf newlib-2.5.0.tar.gz
  fi

  cp -r newlib-2.5.0 newlib-alloy
  NEWLIB_ALLOY=$REPO/build/newlib-alloy
  cd ../toolchain/newlib/patches || exit
  find . -type f -exec patch -u $NEWLIB_ALLOY/{} -i {} \;

  cd ../files || exit
  cp -r newlib ../../../build/newlib-alloy/
  cd ../../../build/newlib-alloy/newlib/libc/sys/alloy || (echo "error copying alloy libc sys folder"; exit)
  pushd $NEWLIB_ALLOY/newlib/libc/sys || exit
  autoconf
  popd

  cd $REPO/build || exit
else
  echo "Found patched newlib at $REPO/build/newlib-alloy"
fi

SYSROOT=$REPO/alloy

mkdir -p $SYSROOT

if [[ ! $(which i686-alloy-ar) ]]; then
  cd $HOME/opt/cross || (echo "no $HOME/opt/cross"; exit)
  ln i686-elf-ar i686-alloy-ar;
  ln i686-elf-as i686-alloy-as;
  ln i686-elf-gcc i686-alloy-gcc;
  ln i686-elf-gcc i686-alloy-cc;
  ln i686-elf-ranlib i686-alloy-ranlib;
  cd $REPO/build || exit
fi

if [[ ! -d $REPO/build/newlib ]]; then
  cd $REPO/build || exit
  mkdir newlib
  cd newlib || exit
  ../../build/newlib-alloy/configure --prefix=/usr --target=i686-alloy
else
  cd $REPO/build/newlib || exit
fi

make -j15 all
make DESTDIR="$SYSROOT" install
cp -r $SYSROOT/usr/i686-alloy/* $SYSROOT/usr

cd $REPO