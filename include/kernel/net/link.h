#ifndef _KERNEL_LINK_H
#define _KERNEL_LINK_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint8_t parts[6];
} MacAddress;

MacAddress new_mac_from_array(uint8_t parts[6]);
MacAddress new_mac(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e, uint8_t f);
MacAddress current_mac();

void send_packet(MacAddress dest, uint16_t frame_type, uint8_t *data_buf, size_t data_len);

#endif