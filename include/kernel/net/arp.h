#ifndef _KERNEL_NET_ARP_H
#define _KERNEL_NET_ARP_H

#include <kernel/net/ip.h>

void arp_request(IPAddress unknown_ip, IPAddress my_ip);

#endif