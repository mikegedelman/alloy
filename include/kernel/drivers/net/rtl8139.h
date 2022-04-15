#ifndef _KERNEL_RTL_8319_H
#define _KERNEL_RTL_8319_H

#include <kernel/net/link.h>
#include <stdint.h>

void rtl_8139_init();
void rtl_8139_tx(void *data, uint32_t len);
MacAddress rtl_8139_get_mac_addr();

#endif