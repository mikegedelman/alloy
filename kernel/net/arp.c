// https://wiki.osdev.org/Address_Resolution_Protocol
#include <kernel/all.h>

#define ARP_ETHERTYPE 0x0806

// typedef struct __attribute__((__packed__)) {

// } ARPPacketHeader;

typedef struct __attribute__((__packed__)) {
	uint16_t hardware_type; // Ethernet: 0x1
	uint16_t protocol_type; // IP: 0x0800
	uint8_t hw_addr_len; // MAC: 6
	uint8_t proto_addr_len; // IPv4 = 4
	uint16_t opcode;

	MacAddress src_hw_addr;
	IPAddress src_proto_addr;
	
	MacAddress dest_hw_addr;
	IPAddress dest_proto_addr;
} ARPPacket;

// ARPPacketHeader new_arp_packet_header(uint16_t opcode) {
// 	ARPPacketHeader header;

// }

// ARPPacket new_arp_packet(uint16_t opcode) {

// 	return packet;
// }

typedef struct {
	IPAddress ip;
	MacAddress mac;
} MacAddressCache;

#define ARP_MAC_CACHE_LEN 255
static MacAddressCache arp_cache[ARP_MAC_CACHE_LEN];

// static bool waiting_for_cache;

// size_t serialize_arp(ARPPacket *packet, uint8_t *buf) {
// 	size_t buf_pos = 0;
// 	memcpy(buf, (void*)&packet->header, sizeof(ARPPacketHeader));
// 	buf_pos += sizeof(ARPPacketHeader);

// 	memcpy(buf + buf_pos, (void*)&packet->src_hw_addr, 6);
// 	buf_pos += 6;

// 	memcpy(buf + buf_pos, (void*)&packet->src_proto_addr, 4);
// 	buf_pos += 4;

// 	memcpy(buf + buf_pos, (void*)&packet->dest_hw_addr, 6);
// 	buf_pos += 6;

// 	memcpy(buf + buf_pos, (void*)&packet->dest_proto_addr, 4);
// 	buf_pos += 4;

// 	return buf_pos;
// }


void arp_request(IPAddress unknown_ip, IPAddress my_ip) {
	uint8_t buf[64];
	MacAddress my_mac = current_mac();
	MacAddress broadcast_mac = new_mac(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

	ARPPacket arp;
	arp.hardware_type = htons(0x1);
	arp.protocol_type = htons(0x0800);
	arp.hw_addr_len = 6;
	arp.proto_addr_len = 4;
	arp.opcode = htons(0x1);
	arp.src_hw_addr = my_mac;
	arp.src_proto_addr = my_ip;
	arp.dest_hw_addr = broadcast_mac;
	arp.dest_proto_addr = unknown_ip;

	size_t data_len = sizeof(ARPPacket);
	memcpy(buf, (void*) &arp, data_len);
	send_packet(broadcast_mac, ARP_ETHERTYPE, buf, data_len);
}

void arp_receive(uint8_t *data) {
	ARPPacket *arp = (ARPPacket*) data;
	print_ip(&arp->dest_proto_addr);
	printf(" is ");
	print_mac(&arp->dest_hw_addr);
	printf("\n");

	static size_t arp_cache_ptr;
	arp_cache[arp_cache_ptr].ip = arp->src_proto_addr;
	arp_cache[arp_cache_ptr].mac = arp->src_hw_addr;
	arp_cache_ptr++;
}

MacAddress *check_cache(IPAddress unknown_ip) {
	for (int i = 0; i < ARP_MAC_CACHE_LEN; i++) {
		MacAddressCache *cache_entry = arp_cache + i;
		if (ip_eq(&cache_entry->ip, &unknown_ip)) {
			return &cache_entry->mac;
		}
	}

	return NULL;
}

MacAddress arp_resolve(IPAddress unknown_ip, IPAddress my_ip) {
	MacAddress *mac = check_cache(unknown_ip);
	if (mac != NULL) {
		return *mac;
	}

	arp_request(unknown_ip, my_ip);
	while (1) {
		mac = check_cache(unknown_ip);
		if (mac != NULL) {
			printf("Resolved\n");
			return *mac;
		}
	}
}