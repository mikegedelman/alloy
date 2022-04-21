#include <kernel/inet.h>

typedef union {
	uint8_t u8[2];
	uint16_t u16;
} u16_union;

typedef union {
	uint8_t u8[4];
	uint32_t u32;
} u32_union;

uint16_t htons(uint16_t hostshort) {
	return ((u16_union) hostshort).u8[1] | ((u16_union) hostshort).u8[0] << 8;
}
uint16_t ntohs(uint16_t netshort) {
	return ((u16_union) netshort).u8[1] | ((u16_union) netshort).u8[0] << 8;
}

uint32_t htonl(uint32_t hostlong) {
	u32_union hl = (u32_union) hostlong;
	return hl.u8[3] | (hl.u8[2] << 8) | (hl.u8[1] << 16) | (hl.u8[0] << 24);
}
uint32_t ntohl(uint32_t netlong) {
	return htonl(netlong);
}