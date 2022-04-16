#ifndef _KERNEL_DHCP_H
#define _KERNEL_DHCP_H

#include <stdint.h>

void dhcp_discover();
void receive_dhcp(uint8_t *data, size_t data_len);

#endif