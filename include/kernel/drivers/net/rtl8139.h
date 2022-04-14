#ifndef _KERNEL_RTL_8319_H
#define _KERNEL_RTL_8319_H

#include <stdint.h>

void rtl_8139_init();
void rtl_8139_tx(void *data, uint32_t len);
void rtl_8139_get_mac_addr(uint8_t *mac_addr);

#endif