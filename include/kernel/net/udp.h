#ifndef _KERNEL_NET_UDP_H
#define _KERNEL_NET_UDP_H

#include <stdint.h>
#include <stddef.h>

void send_udp(IPAddress source_ip, uint16_t source_port, IPAddress dest_ip, uint16_t source_dest, void *data, size_t data_len);

#endif