#ifndef _KERNEL_TCP_H
#define _KERNEL_TCP_H

typedef void (*tcp_listener_fn)(uint8_t*, size_t);

uint16_t tcp_open(IPAddress dest_ip, uint16_t dest_port, tcp_listener_fn fn);
size_t tcp_send(uint16_t host_port, void const *data, size_t data_len);
void receive_tcp(uint8_t const *data, size_t data_len);

#endif