/* Could get really fancy and make this auto-generated */

#ifndef _KERNEL_ALL_H
#define _KERNEL_ALL_H

// Common includes
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Basics
#include <kernel/arch/x86.h>
#include <kernel/term.h>
#include <kernel/stdio.h>
#include <kernel/string.h>
#include <kernel/fs.h>
#include <kernel/interrupts.h>
#include <kernel/cpu.h>
#include <kernel/multiboot.h>
#include <kernel/util.h>
#include <kernel/math.h>

// Memory Management
#include <kernel/mem/physical.h>
#include <kernel/mem/virtual.h>
#include <kernel/mem/heap.h>

// Drivers
#include <kernel/drivers/uart16550.h>
#include <kernel/drivers/pic8259.h>
#include <kernel/drivers/pci.h>
#include <kernel/drivers/ata.h>
#include <kernel/drivers/net/rtl8139.h> // other net drivers are unfinished

// Filesystem
#include <kernel/fs/volume.h>
#include <kernel/fs/fat.h>

// Network
#include <kernel/inet.h>
#include <kernel/net/link.h>
#include <kernel/net/ip.h>
#include <kernel/net/arp.h>
#include <kernel/net/udp.h>
#include <kernel/net/tcp.h>
#include <kernel/net/dhcp.h>

// ELF support
#include <kernel/proc/elf.h>

#endif