#!/bin/bash

AUTOCONF_VERSION="2.65"
AUTOMAKE_VERSION="1.11"

REPO=$(pwd)
mkdir -p build
cd build || exit

CPU_CORES=1
if [[ $OSTYPE == 'darwin'* ]]; then
  CPU_CORES=$(sysctl -n hw.logicalcpu)
elif [[ $OSTYPE == 'linux'* ]]; then
  CPU_CORES=$(nproc --all)
fi

echo "Detected $CPU_CORES CPU core(s)"
sleep 1

TOOLS_PREFIX="$REPO/build/newlib-tools"
mkdir -p $TOOLS_PREFIX

export PATH="$TOOLS_PREFIX/bin:$PATH"

if [[ ! $(which i686-alloy-ar) ]]; then
  cd $HOME/opt/cross/bin || (echo "no $HOME/opt/cross"; exit)
  ln i686-elf-ar i686-alloy-ar;
  ln i686-elf-as i686-alloy-as;
  ln i686-elf-gcc i686-alloy-gcc;
  ln i686-elf-gcc i686-alloy-cc;
  ln i686-elf-ranlib i686-alloy-ranlib;

  if [[ ! $(which i686-alloy-ar) ]]; then
    echo "Unable to properly copy i686-elf-* files to i686-alloy-* files. Exiting."
    exit
  fi

  cd $REPO/build || exit
fi

if [[ ! -f $TOOLS_PREFIX/bin/autoconf ]]; then

  curl https://ftp.gnu.org/gnu/autoconf/autoconf-$AUTOCONF_VERSION.tar.gz -o autoconf-$AUTOCONF_VERSION.tar.gz
  tar xf autoconf-$AUTOCONF_VERSION.tar.gz
  rm -rf autoconf
  mkdir autoconf
  cd autoconf || exit
  ../autoconf-$AUTOCONF_VERSION/configure --prefix="$TOOLS_PREFIX"
  make && make install
  cd $REPO/build || exit
else
  echo "Found autoconf: $TOOLS_PREFIX/bin/autoconf"
fi

if [[ ! -f $TOOLS_PREFIX/bin/automake ]]; then
  curl https://ftp.gnu.org/gnu/automake/automake-$AUTOMAKE_VERSION.tar.gz -o automake-$AUTOMAKE_VERSION.tar.gz
  tar xf automake-$AUTOMAKE_VERSION.tar.gz
  rm -rf automake
  mkdir automake
  cd automake || exit
  ../automake-$AUTOMAKE_VERSION/configure --prefix="$TOOLS_PREFIX"
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
  cd $REPO/toolchain/newlib/patches || exit
  find . -type f -exec patch -u $NEWLIB_ALLOY/{} -i {} \;

  cd $REPO/toolchain/newlib/files || exit
  cp -r ./* $REPO/build/newlib-alloy/

  cd $REPO/build/newlib-alloy/newlib/libc/sys/alloy || (echo "error copying alloy libc sys folder"; exit)
  autoreconf || exit

  cd $REPO/build/newlib-alloy/newlib/libc/sys || exit
  autoreconf || exit

  cd $REPO/build || exit
else
  echo "Found patched newlib at $REPO/build/newlib-alloy"
  cd $REPO/toolchain/newlib/files || (echo "error"; exit)
  cp -r ./* $REPO/build/newlib-alloy/
  echo "Copied files from $REPO/toolchain/newlib/files"
  echo "Don't forget to run autoreconf if you update Makefile.am"
fi

echo $PATH

SYSROOT=$REPO/alloy

mkdir -p $SYSROOT

if [[ -d $REPO/build/newlib ]]; then
  rm -rf $REPO/build/newlib
fi

mkdir $REPO/build/newlib
cd $REPO/build/newlib || exit
../../build/newlib-alloy/configure --prefix=/usr --target=i686-alloy

make -j$CPU_CORES all
make DESTDIR="$SYSROOT" install
cp -r $SYSROOT/usr/i686-alloy/* $SYSROOT/usr

if [[ ! -f $SYSROOT/usr/lib/crt0.o ]]; then
  echo "*** WARNING: Couldn't find /usr/lib/crt0.o in SYSROOT."
fi

if [[ ! -f $SYSROOT/usr/lib/libc.a ]]; then
  echo "*** WARNING: Couldn't find /usr/lib/libc.a in SYSROOT."
fi

cd $REPO
