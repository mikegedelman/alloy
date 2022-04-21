#include <kernel/all.h>

#define IPV4_ETHERTYPE 0x0800

static uint8_t ip_buf[1500];

typedef struct {
	IPAddress my_ip;
} IPState;

enum IPProtocolType {
	IP_PROTO_UDP = 0x11,
	IP_PROTO_TCP = 0x06
};

static IPState ip_state;

void set_my_ip(IPAddress addr) {
	ip_state.my_ip = addr;
}	

IPAddress get_my_ip() {
	return ip_state.my_ip;
}

bool ip_eq(IPAddress const *a, IPAddress const *b) {
	return memcmp((uint8_t*)a, (uint8_t*)b, 4) == 0;
}

void print_ip(IPAddress const *ip) {
		printf("%x.%x.%x.%x", ip->parts[0], ip->parts[1], ip->parts[2], ip->parts[3]);
}

IPAddress new_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
	IPAddress ip;
	ip.parts[0] = a;
	ip.parts[1] = b;
	ip.parts[2] = c;
	ip.parts[3] = d;
	return ip;
}

typedef struct __attribute__((__packed__)) {
	uint8_t version; // 4 - (4 bits)
	uint8_t header_len; // 5 unless options used (4 bits)
	uint8_t dscp_and_ecn; // prob just 0 (6/2 bits)
	uint16_t message_length;
	uint16_t identification;
	uint8_t flags; // 3 bits
	uint16_t fragment_offset; // 13 bits
	uint8_t ttl;
	uint8_t protocol; // https://en.wikipedia.org/wiki/List_of_IP_protocol_numbers
	// uint16_t header_checksum; // calculated when serializing
	IPAddress source;
	IPAddress dest;
	// options?
} IPHeader;

IPHeader new_ip_header(IPAddress source, IPAddress dest, uint8_t protocol, size_t message_length) {
	IPHeader header;
	header.version = 4;
	header.header_len = 5;
	header.dscp_and_ecn = 0;
	header.message_length = message_length + 20;
	header.identification = 1;
	header.flags = 0;
	header.fragment_offset = 0;
	header.ttl = 64;
	header.protocol = protocol; // UDP = 0x11
	header.source = source;
	header.dest = dest;
	return header;
}

uint16_t compute_checksum(uint16_t *addr, size_t count) {
  register uint32_t sum = 0;
  while (count > 1) {
    sum += htons(*addr);
    addr++;
    count -= 2;
  }
  //if any bytes left, pad the bytes and add
  if(count > 0) {
    sum += ((*addr)&htons(0xFF00));
  }
  //Fold sum to 16 bits: add carrier to result
  while (sum>>16) {
      sum = (sum & 0xffff) + (sum >> 16);
  }
  //one's complement
  sum = ~sum;
  return ((uint16_t) htons(sum));
}

void send_ip(IPAddress source, IPAddress dest, uint8_t protocol, void *data, size_t data_len) {
	IPHeader header = new_ip_header(source, dest, protocol, data_len);
	if (data_len > 1450) {
		printf("IP Fragmentation not supported - dropping packet for data_len of size %i\n", data_len);
		return;
	}

	// ip_buf[0] = header.version | (header.header_len << 4);
	ip_buf[0] = header.header_len | (header.version << 4);
	ip_buf[1] = header.dscp_and_ecn;
	uint16_t be_message_length = htons(header.message_length);
	memcpy(ip_buf + 2, &be_message_length, 2);
	ip_buf[4] = header.identification;
	ip_buf[6] = header.fragment_offset | (header.flags << 3);
	ip_buf[8] = header.ttl;
	// printf("protocol: %x\n", header.protocol);
	ip_buf[9] = header.protocol;
	ip_buf[10] = 0; // set checksum to 0 initially
	ip_buf[11] = 0;
	memcpy(ip_buf + 12, (void*)&header.source, 4);
	memcpy(ip_buf + 16, (void*)&header.dest, 4);
	memcpy(ip_buf + 20, data, data_len);

	uint16_t final_checksum_be = compute_checksum((uint16_t*)ip_buf, 20);
	memcpy(ip_buf + 10, &final_checksum_be, 2);

	// I'm not so sure about this...
	IPAddress broadcast_ip = new_ip(255, 255, 255, 255);
	MacAddress target_mac;
	if (ip_eq(&dest, &broadcast_ip)) {
		target_mac = new_mac(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
	} else {
		IPAddress router_ip = dhcp_get_router_ip();
		target_mac = arp_resolve(router_ip, source);
	}
	send_packet(target_mac, IPV4_ETHERTYPE, ip_buf, data_len + 20);
}

void receive_ip(uint8_t *data, size_t data_len) {
	// IPHeader *header = (IPHeader*) data; TODO make this work

	// Maybe check data_len against the IP header's message length?
	uint8_t protocol = data[9];
	size_t message_length = ntohs(*(uint16_t*)(data + 2)) - 20;

	switch (protocol) {
		case IP_PROTO_UDP:
			receive_udp(data + 20, message_length);
			break;
		case IP_PROTO_TCP:
			printf("IP: data_len: %x\n", message_length);
			receive_tcp(data + 20, message_length);
			break;
		default:
			printf("Unsupported protocol %x. Dropping packet.\n", protocol);
	}
}