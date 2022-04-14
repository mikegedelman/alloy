// For now, tightly coupled to the rtl_8139 driver.

#include <kernel/drivers/net/rtl8139.h>

typedef struct {
	uint8_t mac_address[6];
} NetInfo;

NetInfo get_mac_address() {
	NetInfo info;
	rtl_8139_get_mac_addr(info.mac_address);
	return info;
}

void send_packet(void *data, uint32_t len) {
	rtl_8139_tx(data, len);
}
