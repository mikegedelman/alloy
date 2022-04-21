#include <kernel/all.h>

#define TCP_IP_PROTOCOL 0x06



typedef struct __attribute__((__packed__)) {
	uint16_t source_port;
	uint16_t dest_port;
	uint32_t seq_num;
	uint32_t ack_num;
	uint16_t do_rsv_flags;
	uint16_t window;
	uint16_t checksum;
	uint16_t urgent_pointer;
	// uint8_t *options;
} TCPHeader;

typedef struct {
	// TCP state
	int tcp_state;
	IPAddress dest_ip;
	uint16_t source_port;
	uint16_t dest_port;
	uint32_t my_seq_num;
	uint32_t server_seq_num;

	uint32_t server_ack;

	// Listener
	tcp_listener_fn listener;
} TCPConfig;

static TCPConfig tcp_config[0xFFFF];
static uint16_t tcp_outbound_port = 51673;


#define FIN (1 << 0)
#define SYN (1 << 1)
#define RST (1 << 2)
#define PSH (1 << 3)
#define ACK (1 << 4)
#define URG (1 << 5)

enum TCPState {
	CLOSED,
	LISTEN,
	SYN_SENT,
	SYN_RECEIVED,
	ESTABLISHED,
	FIN_WAIT_1,
	FIN_WAIT_2,
	CLOSE_WAIT,
	CLOSING,
	LAST_ACK,
	TIME_WAIT,
	NEW // this is my own, just to indicate that nothing has happened on the port yet
};


typedef struct  __attribute__((__packed__)) {
	IPAddress src_ip;
	IPAddress dest_ip;
	uint16_t ip_protocol;
	uint16_t tcp_length;
} PseudoIPHeader;

uint16_t tcp_checksum(TCPConfig const *config, uint8_t const *tcp_buf, size_t data_len) {
	PseudoIPHeader pseudo_header;
	pseudo_header.dest_ip = config->dest_ip;
	pseudo_header.src_ip = get_my_ip();
	pseudo_header.ip_protocol = htons(TCP_IP_PROTOCOL);
	pseudo_header.tcp_length = htons(data_len);

	uint32_t sum = 0;
	size_t count = sizeof(PseudoIPHeader);
	uint16_t *buf_ptr = (uint16_t*) &pseudo_header;
	uint16_t cur_num;
	while (count > 1) {
		cur_num = htons(*buf_ptr);
		sum += cur_num;
		buf_ptr++;
		count -= 2;
	}

	// Now add up all the 16s in the TCP segment
	count = data_len;
	buf_ptr = (uint16_t*) tcp_buf;
	while (count > 1) {
		cur_num = htons(*buf_ptr);
		sum += cur_num;
		buf_ptr++;
		count -= 2;
	}
	// if any bytes left, pad the bytes and add
	if(count > 0) {
		sum += (htons(*buf_ptr & 0xFF));
	}
	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}
	// one's complement
	sum = ~sum;

	return htons(sum);
}

void build_tcp_header(
	TCPHeader *header,
	TCPConfig const *config,
	uint32_t seq_num,
	uint32_t ack_num,
	uint8_t header_len,
	uint16_t flags
	// Skip window, urgent pointer for now
) {
	header->source_port = htons(config->source_port);
	header->dest_port = htons(config->dest_port);
	header->seq_num = htonl(seq_num);
	header->ack_num = htonl(ack_num);
	header->do_rsv_flags = htons((header_len << 12) | flags);
	header->window = htons(1460); // Don't support packet buffering lol
	header->checksum = 0;
	header->urgent_pointer = 0;
}

void tcp_syn(TCPConfig *config) {
	config->my_seq_num = 1;
	config->server_seq_num = 0;

	size_t data_len = sizeof(TCPHeader) + 4; // +4 for options
	uint8_t tcp_buf[data_len];

	TCPHeader *header = (TCPHeader*) tcp_buf;
	printf("sending seq_num=%x, ack_num=%x\n", config->my_seq_num, 0);
	build_tcp_header(header, config, config->my_seq_num, 0, 6, SYN);

	// TODO don't finagle this
	tcp_buf[sizeof(TCPHeader) + 0] = 0x02;
	tcp_buf[sizeof(TCPHeader) + 1] = 0x04;
	tcp_buf[sizeof(TCPHeader) + 2] = 0x05;
	tcp_buf[sizeof(TCPHeader) + 3] = 0xb4;

	header->checksum = tcp_checksum(config, tcp_buf, data_len);

	IPAddress my_ip = get_my_ip();
	send_ip(my_ip, config->dest_ip, TCP_IP_PROTOCOL, tcp_buf, data_len);

	config->tcp_state = SYN_SENT;
}

void tcp_ack(TCPConfig *config, uint16_t extra_flags) {
	size_t data_len = sizeof(TCPHeader);
	uint8_t tcp_buf[data_len];

	TCPHeader *header = (TCPHeader*) tcp_buf;
	printf("sending seq_num=%x, ack_num=%x\n", config->my_seq_num, config->server_seq_num);
	build_tcp_header(header, config, config->my_seq_num, config->server_seq_num, 5, ACK | extra_flags);
	header->checksum = tcp_checksum(config, tcp_buf, data_len);

	IPAddress my_ip = get_my_ip();
	send_ip(my_ip, config->dest_ip, TCP_IP_PROTOCOL, tcp_buf, data_len);

	config->tcp_state = ESTABLISHED;
}

void receive_tcp(uint8_t const *data, size_t data_len) {
	TCPHeader *header = (TCPHeader*)data;
	uint16_t host_port = ntohs(header->dest_port);
	
	TCPConfig *config = &tcp_config[host_port];
	if (config->tcp_state == CLOSED) {
		printf("Dropping %x bytes on TCP port %x: port closed\n", data_len, host_port);
		return;
	}

	uint16_t flags = 0x1FF & ntohs(header->do_rsv_flags);
	printf("flags: %x\n", flags);

	if (flags & (SYN | ACK) && config->tcp_state == SYN_SENT) {
		config->my_seq_num++;
		config->server_seq_num = ntohl(header->seq_num) + 1;
		tcp_ack(config, 0);
		// config->server_seq_num++;
	}
	else if (config->tcp_state == CLOSE_WAIT) {
		if (flags & ACK) {
			// don't verify seq numbers lol just close it
			config->tcp_state = CLOSED;
			printf("TCP connection to ");
			print_ip(&config->dest_ip);
			printf(":%x closed.\n", config->dest_port);
		}
	}
	else if (config->tcp_state == ESTABLISHED) {
		uint8_t tcp_header_size = (htons(header->do_rsv_flags) >> 12) * 4;

		if (flags & ACK) {
			config->server_ack = htons(header->ack_num);
		}
		if (flags & PSH) {
			printf("flags & PSH %x\n", flags & PSH);
			printf("TCP header size: %x\ndata_len: %x\n", tcp_header_size, data_len);
			size_t payload_len = data_len - tcp_header_size;
			printf("payload len: %x\n", payload_len);
			// printf("config->seq_num: %x\n", config->seq_num);
			printf("server seq num: %x -> ", config->server_seq_num);
			config->server_seq_num += payload_len;
			printf("%x\n", config->server_seq_num);
			// uint32_t seq_num = ntohl(header->seq_num);
			tcp_ack(config, 0);

			config->listener(data + tcp_header_size, payload_len);
			// config->seq_num = config->seq_num + payload_len + 1;
		}
		if (flags & FIN) {
			config->server_seq_num++;
			tcp_ack(config, FIN);
			config->tcp_state = CLOSE_WAIT;
		}
	}
}


uint16_t tcp_open(IPAddress dest_ip, uint16_t dest_port, tcp_listener_fn fn) {
	uint16_t host_port = tcp_outbound_port;

	// TODO check if the outbound port we're trying to select is in use,
	// if so pick another one.
	TCPConfig *config = &tcp_config[tcp_outbound_port];
	tcp_outbound_port++;

	config->tcp_state = NEW;
	config->dest_ip = dest_ip;
	config->dest_port = dest_port;
	config->source_port = host_port;
	config->server_ack = 0;
	config->listener = fn;

	tcp_syn(config);
	while (1) {
		if (config->tcp_state == ESTABLISHED) {
			break;
		}
		io_wait();
	}

	return host_port;
}

bool tcp_is_open(uint16_t host_port) {
	if (tcp_config[host_port].tcp_state != CLOSED) return true;

	return false;
}

size_t tcp_send(uint16_t host_port, void const *data, size_t data_len) {
	if (tcp_config[host_port].tcp_state != ESTABLISHED) return 0;
	// TODO handle data that doesn't fit in one packet lol
	size_t total_len = data_len + 20;
	uint8_t tcp_buf[total_len];

	TCPHeader *header = (TCPHeader*) tcp_buf;
	TCPConfig *config = &tcp_config[host_port];
	printf("sending seq_num=%x, ack_num=%x\n", config->my_seq_num, config->server_seq_num);
	build_tcp_header(header, config, config->my_seq_num, config->server_seq_num, 5, ACK | PSH);
	memcpy(tcp_buf + sizeof(TCPHeader), data, data_len);
	header->checksum = tcp_checksum(config, tcp_buf, total_len);

	// printf("my_seq_num: %x, add %x\n", config->my_seq_num, data_len);
	config->my_seq_num += data_len;
	IPAddress my_ip = get_my_ip();
	send_ip(my_ip, config->dest_ip, TCP_IP_PROTOCOL, tcp_buf, total_len);
}
