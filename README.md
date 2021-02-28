# alloy

An osdev experiment inspired by https://os.phil-opp.com/, but with less magic.

Phil's series of blog posts on writing an OS in Rust is _awesome_.
He has some of the best explanations on interrupts, paging, etc, I can
find - as a newbie, at least.

This project is inspired by his blog-os, but with fewer Rust
dependencies hiding the lower-level details from you. This has some
advantages and some drawbacks. The primary advantage (and motivation)
is to delve lower into the details: learning about the most bare-metal
code is what interests me about osdev. Also, with this setup, it's
easier to farm out things to asm, C, C++, anything that can be
statically linked to, since we already have a linker set up and
ready to go. The downsides are: needing a cross-linker; integrating
Cargo into a Makefile (there must be a better way than what I have);
more complexity.

My primary goal with this project is to provide a resource to learn more
about osdev.


## Major differences from blog-os

* Target is `i686`, not `x86_64`: most osdev tutorials out there are
are for 32-bit x86, and removing 64-bit support slightly reduces the
complexity, since we've added a lot more complexity by writing
lower-level code.

* Higher half kernel: we're loaded at VMA `0xC0000000`.

* Minimizing dependencies has been prioritized: most of the dependencies
I've incurred are to support code I've ripped from Phil's blog. I may
go back and remove even those in the future.

* Testing is organized pretty differently: I'll chronicle this in detail
somewhere else, but basically since we're doing other stuff to Cargo's
output before running it, I can't the get normal Rust test methods
working. I've worked around it using a feature flag for the time being.


## Roadmap

- [x] Boot using a multiboot header
- [x] Print text to VGA buffer
- [x] Ability to write tests
- [x] Load GDT
- [x] Enable interrupts
- [x] Handle keyboard input
- [x] Higher half kernel
- [] Handle CPU exceptions, double faults
- [] Memory management
- [] Custom bootloader

### Prerequisites

If you're here to learn about osdev, you should read [osdev.org Required Knowledge](https://wiki.osdev.org/Required_Knowledge) first.
Their [Bare Bones](https://wiki.osdev.org/Bare_bones) is a good place
to start with osdev. Once you have a basic grasp of what's going on
there, the stuff going on here will start to make more sense.

DISCLAIMER: This project is a product of me learning osdev as I go.
Lots of here has been copied from the internet and tweaked by me
to get it working. So, I'm not presenting this code as some paragon of
operating system design; it's more that I'm trying to provide
a beginner-level osdev resource that has code written in Rust
instead of C.


## Usage

### Install dependencies

This has only been tested on MacOS currently but probably works on Linux.
Install commands are for MacOS; if you're a Linux user I'm sure you
can figure out where to get these packages.

**qemu, nasm, make**

```
brew install qemu nasm
xcode-select --install  # This *should* install make. You could also
                        # install it using brew
```

**rust**

```
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

See: https://rustup.rs/

**docker**

Download Docker for Mac: https://hub.docker.com/editions/community/docker-ce-desktop-mac

The Makefile references an image called `mikegedelman/alloy:master` that
will be kept up to date with the `master` branch in this repo. [Docker
Hub repo](https://hub.docker.com/repository/docker/mikegedelman/alloy)

If you really don't want to install Docker, it's only used for
the linker right now. You could also compile binutils yourself:

⚠️ Optional ⚠️
```
export PREFIX="$HOME/opt/cross"  # you might want to change this:
export TARGET=i686-elf           # I use $HOME/.local
export PATH="$PREFIX/bin:$PATH"

mkdir $HOME/src
cd $HOME/src
tar xvf binutils-2.36.tar.xz
mkdir build-binutils
cd build-binutils
../binutils-2.36/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install
```

If you do this, also change LD in the Makefile to no longer use docker.
BONUS: a locally-compiled binutils runs much quicker than running via
docker every time.


### Setup

```
git clone https://github.com/mikegedelman/alloy.git
make run
```

You should see a QEMU window pop up with a message like `Welcome to alloy`.
