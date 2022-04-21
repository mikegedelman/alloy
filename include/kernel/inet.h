#ifndef _KERNEL_INET_H
#define _KERNEL_INET_H

#include <stdint.h>

uint16_t htons(uint16_t hostshort);
uint16_t ntohs(uint16_t netshort);
uint32_t htonl(uint32_t hostlong);
uint32_t ntohl(uint32_t netlong);

#endif
