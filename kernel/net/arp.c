// https://wiki.osdev.org/Address_Resolution_Protocol
#include <kernel/string.h>
#include <kernel/net/link.h>
#include <kernel/net/ip.h>
#include <kernel/net/arp.h>
#include <kernel/inet.h>

#define ARP_ETHERTYPE 0x0806

typedef struct __attribute__((__packed__)) {
	uint16_t hardware_type; // Ethernet: 0x1
	uint16_t protocol_type; // IP: 0x0800
	uint8_t hw_addr_len; // MAC: 6
	uint8_t proto_addr_len; // IPv4 = 4
	uint16_t opcode;
} ARPPacketHeader;

typedef struct {
	ARPPacketHeader header;

	MacAddress src_hw_addr;
	IPAddress src_proto_addr;
	
	MacAddress dest_hw_addr;
	IPAddress dest_proto_addr;
} ARPPacket;

ARPPacketHeader new_arp_packet_header(uint16_t opcode) {
	ARPPacketHeader header;
	header.hardware_type = htons(0x1);
	header.protocol_type = htons(0x0800);
	header.hw_addr_len = 6;
	header.proto_addr_len = 4;
	header.opcode = htons(opcode);
	return header;
}

ARPPacket new_arp_packet(uint16_t opcode) {
	ARPPacket packet;
	packet.header = new_arp_packet_header(opcode);
	return packet;
}

size_t serialize_arp(ARPPacket *packet, uint8_t *buf) {
	size_t buf_pos = 0;
	memcpy(buf, (void*)&packet->header, sizeof(ARPPacketHeader));
	buf_pos += sizeof(ARPPacketHeader);

	memcpy(buf + buf_pos, (void*)&packet->src_hw_addr, 6);
	buf_pos += 6;

	memcpy(buf + buf_pos, (void*)&packet->src_proto_addr, 4);
	buf_pos += 4;

	memcpy(buf + buf_pos, (void*)&packet->dest_hw_addr, 6);
	buf_pos += 6;

	memcpy(buf + buf_pos, (void*)&packet->dest_proto_addr, 4);
	buf_pos += 4;

	return buf_pos;
}


void arp_request(IPAddress unknown_ip, IPAddress my_ip) {
	uint8_t buf[64];
	MacAddress my_mac = current_mac();
	MacAddress broadcast_mac = new_mac(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

	ARPPacket arp = new_arp_packet(0x001);
	arp.src_hw_addr = my_mac;
	arp.src_proto_addr = my_ip;
	arp.dest_hw_addr = broadcast_mac;
	arp.dest_proto_addr = unknown_ip;

	size_t data_len = serialize_arp(&arp, buf);
	send_packet(broadcast_mac, ARP_ETHERTYPE, buf, data_len);
	// send_packet(buf, packet_len);
}