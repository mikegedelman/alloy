# Build a gcc cross-compiler for i686-elf
# Commands mostly from here https://wiki.osdev.org/GCC_Cross-Compiler
FROM alpine

RUN apk --no-cache add curl build-base texinfo mpfr-dev mpc1-dev gmp-dev nasm

ENV PREFIX="$HOME/opt/cross"
ENV TARGET=i686-elf
ENV PATH="$PREFIX/bin:$PATH"

RUN curl https://ftp.gnu.org/gnu/binutils/binutils-2.38.tar.xz -o binutils-2.38.tar.xz
RUN curl http://gnu.mirrors.hoobly.com/gcc/gcc-11.2.0/gcc-11.2.0.tar.xz -o gcc-11.2.0.tar.xz

# Build binutils
RUN tar xf binutils-2.38.tar.xz
RUN mkdir build-binutils
RUN cd build-binutils
RUN ../binutils-2.38/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
RUN make -j4
RUN make install

# Build GCC
RUN which -- $TARGET-as || echo $TARGET-as is not in the PATH
RUN tar xf gcc-11.2.0.tar.xz
RUN mkdir build-gcc
RUN cd build-gcc
RUN make distclean
RUN ../gcc-11.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c --without-headers
RUN make -j4 all-gcc
RUN make all-target-libgcc
RUN make install-gcc
RUN make install-target-libgcc

RUN apk --no-cache add coreutils

# RUN curl https://ftp.gnu.org/gnu/automake/automake-1.11.tar.gz -o automake-1.11.tar.gz
# RUN tar xf automake-1.11.tar.gz
# RUN curl https://ftp.gnu.org/gnu/autoconf/autoconf-2.65.tar.gz -o autoconf-2.65.tar.gz
# RUN tar xf autoconf-2.65.tar.gz
# 
# RUN apk --no-cache add m4
# RUN ls -l
# 
# # RUN mkdir build-autoconf
# # RUN cd build-autoconf
# WORKDIR /build-autoconf
# RUN ../autoconf-2.65/configure --prefix="$PREFIX"
# RUN make -j4
# RUN make install
# 
# WORKDIR /build-automake
# RUN ../automake-1.11/configure --prefix="$PREFIX"
# # RUN make
# # RUN make install
# # RUN cd ..
# 
# WORKDIR /
# RUN curl https://ftp.gnu.org/gnu/autoconf/autoconf-2.64.tar.xz -o autoconf-2.64.tar.xz
# RUN tar xf autoconf-2.64.tar.xz
# 
# WORKDIR /build-autoconf-264
# RUN mkdir /autoconf264
# RUN ../autoconf-2.64/configure --prefix="/autoconf264"
# RUN make
# RUN make install

