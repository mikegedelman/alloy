#include <kernel/net/link.h>
#include <kernel/net/ip.h>
#include <kernel/inet.h>
#include <kernel/stdio.h>
#include <kernel/string.h>

#define IPV4_ETHERTYPE 0x0800

static uint8_t ip_buf[1500];

typedef struct {
	IPAddress my_ip;
} IPState;

static IPState ip_state;

void set_my_ip(IPAddress addr) {
	ip_state.my_ip = addr;
}	

IPAddress get_my_ip() {
	return ip_state.my_ip;
}

void print_ip(uint8_t *ip) {
		printf("%x.%x.%x.%x", ip[0], ip[1], ip[2], ip[3]);
}

IPAddress new_ip(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
	IPAddress ip;
	ip.parts[0] = a;
	ip.parts[1] = b;
	ip.parts[2] = c;
	ip.parts[3] = d;
	return ip;
}

typedef struct {
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
    sum += * addr++;
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
  return ((uint16_t)sum);
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
	printf("protocol: %x\n", header.protocol);
	ip_buf[9] = header.protocol;

	uint16_t final_checksum_be = compute_checksum((uint16_t*)ip_buf, 10);

	memcpy(ip_buf + 10, &final_checksum_be, 2);
	memcpy(ip_buf + 12, (void*)&header.source, 4);
	memcpy(ip_buf + 16, (void*)&header.dest, 4);
	memcpy(ip_buf + 20, data, data_len);

	MacAddress broadcast_mac = new_mac(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
	send_packet(broadcast_mac, IPV4_ETHERTYPE, ip_buf, data_len + 20);
}

void receive_ip(uint8_t *data, size_t data_len) {
	IPAddress *source = (IPAddress*)(data + 12);
	IPAddress *dest = (IPAddress*)(data + 16);

	printf("source IP: ");
	print_ip(source);
	printf("\n");
	printf("dest IP: ");
	print_ip(dest);
	printf("\n");

	uint8_t protocol = data[9];
	printf("protocol: %x\n", protocol);

	switch (protocol) {
		case 0x11:
			receive_udp(data + 20, data_len - 20);
			break;
		default:
			printf("Unsupported protocol %x. Dropping packet.\n", protocol);
	}
}