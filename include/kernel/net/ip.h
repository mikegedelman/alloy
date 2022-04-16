#ifndef _KERNEL_NET_IP_H
#define _KERNEL_NET_IP_H

#include <stdint.h>

typedef struct __attribute__((__packed__)) {
	uint8_t parts[4];
} IPAddress;

IPAddress new_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d);
void send_ip(IPAddress source, IPAddress dest, uint8_t protocol, void *data, size_t data_len);
void print_ip(uint8_t *ip);
void receive_ip(uint8_t *data, size_t data_len);

#endif