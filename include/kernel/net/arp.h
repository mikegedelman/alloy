#ifndef _KERNEL_NET_ARP_H
#define _KERNEL_NET_ARP_H

#include <kernel/net/ip.h>

MacAddress arp_resolve(IPAddress unknown_ip, IPAddress my_ip);
void arp_receive(uint8_t *data);

#endif