#include <stdbool.h>

#include <kernel/string.h>
#include <kernel/net/link.h>
#include <kernel/inet.h>

// For now, tightly coupled to the rtl_8139 driver.
#include <kernel/drivers/net/rtl8139.h>

static MacAddress _rtl_mac;
static bool _did_init_mac = false;

static uint8_t packet_buf[1518];

MacAddress new_mac_from_array(uint8_t parts[6]) {
	MacAddress mac;
	mac.parts[0] = parts[0];
	mac.parts[1] = parts[1];
	mac.parts[2] = parts[2];
	mac.parts[3] = parts[3];
	mac.parts[4] = parts[4];
	mac.parts[5] = parts[5];
	return mac;
}

MacAddress new_mac(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f) {
	MacAddress mac;
	mac.parts[0] = a;
	mac.parts[1] = b;
	mac.parts[2] = c;
	mac.parts[3] = d;
	mac.parts[4] = e;
	mac.parts[5] = f;
	return mac;
}

MacAddress current_mac() {
	if (!_did_init_mac) {
		_rtl_mac = rtl_8139_get_mac_addr();
		_did_init_mac = true;
	}

	return _rtl_mac;
}

// static uint8_t packet_buf[1614]; // todo proper length

void send_packet(MacAddress dest, uint16_t frame_type, uint8_t *data_buf, size_t data_len) {
	size_t buf_pos = 0;
	memcpy(packet_buf, &dest, 6);
	buf_pos += 6;

	MacAddress my_mac = current_mac();
	memcpy(packet_buf + buf_pos, &my_mac, 6);
	buf_pos += 6;

	uint16_t be_frame_type = htons(frame_type);
	memcpy(packet_buf + buf_pos, &be_frame_type, 2);
	buf_pos += 2;

	memcpy(packet_buf + buf_pos, data_buf, data_len);
	buf_pos += data_len;

	if (buf_pos < 64) {
		for (int i = buf_pos; i < 64; i++) {
			packet_buf[i] = 0;
		}
		buf_pos = 64;
	}

	rtl_8139_tx(packet_buf, buf_pos);
}