#include <kernel/inet.h>

typedef union {
	uint8_t u8[2];
	uint16_t u16;
} u16_union;

uint16_t htons(uint16_t hostshort) {
	return ((u16_union) hostshort).u8[1] | ((u16_union) hostshort).u8[0] << 8;
}
uint16_t ntohs(uint16_t netshort) {
	return ((u16_union) netshort).u8[1] | ((u16_union) netshort).u8[0] << 8;
}