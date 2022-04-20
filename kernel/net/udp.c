#include <kernel/all.h>

#define UDP_IP_PROTOCOL 0x11

static uint8_t udp_buf[1500];

static void (*registered_listeners[0xFFFF]) (uint8_t*, size_t);

typedef struct  __attribute__((__packed__)) {
	uint16_t source_port;
	uint16_t destination_port;
	uint16_t message_length;
	uint16_t checksum;
} UDPHeader;

void send_udp(IPAddress source_ip, uint16_t source_port, IPAddress dest_ip, uint16_t dest_port, void *data, size_t data_len) {
	UDPHeader header;
	header.source_port = htons(source_port);
	header.destination_port = htons(dest_port);
	header.message_length = htons(data_len + 8);
	header.checksum = 0;

	memcpy(udp_buf, (void*)&header, 8);
	memcpy(udp_buf + 8, data, data_len);

	send_ip(source_ip, dest_ip, UDP_IP_PROTOCOL, udp_buf, data_len + 8);
}

void receive_udp(uint8_t *data, size_t data_len) {
	UDPHeader *header = (UDPHeader*)data;
	uint16_t port = ntohs(header->destination_port);

	printf("Received %x bytes on UDP port %x\n", ntohs(header->message_length), port);
	void (*listener)(uint8_t*, size_t) = registered_listeners[port];
	if (listener == NULL) {
		printf("No listener on UDP port %x - dropping packet.\n");
		return;
	}

	listener(data + sizeof(UDPHeader), data_len - sizeof(UDPHeader));
	// receive_dhcp(data + sizeof(UDPHeader), data_len - sizeof(UDPHeader));
}

void register_udp_listener(uint16_t port, void (*fn)(uint8_t*, size_t)) {
	registered_listeners[port] = fn;
}