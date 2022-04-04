# Build a gcc cross-compiler for i686-elf
# Commands mostly from here https://wiki.osdev.org/GCC_Cross-Compiler
FROM alpine

RUN apk --no-cache add curl build-base texinfo mpfr-dev mpc1-dev gmp-dev nasm

ENV PREFIX="$HOME/opt/cross"
ENV TARGET=i686-elf
ENV PATH="$PREFIX/bin:$PATH"

RUN curl https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.xz -o binutils-2.38.tar.xz
RUN curl http://gnu.mirrors.hoobly.com/gcc/gcc-11.2.0/gcc-11.2.0.tar.xz -o gcc-11.2.0.tar.xz

RUN apt-get install -y xz-utils build-essential libgmp3-dev libmpc-dev libmpfr-dev grub2-common grub-pc-bin xorriso

# Build binutils
RUN mkdir $HOME/src
RUN cd $HOME/src
RUN tar xvf binutils-2.38.tar.xz
RUN mkdir build-binutils
RUN cd build-binutils
RUN ../binutils-2.38/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
RUN make -j4
RUN make install

# Build GCC
RUN cd $HOME/src
RUN which -- $TARGET-as || echo $TARGET-as is not in the PATH
RUN tar xvf gcc-11.2.0.tar.xz
RUN mkdir build-gcc
RUN cd build-gcc
RUN make distclean
RUN ../gcc-11.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
RUN make -j4 all-gcc
RUN make all-target-libgcc
RUN make install-gcc
RUN make install-target-libgcc
