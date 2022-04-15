#include <kernel/inet.h>

uint16_t htons(uint16_t hostshort) {
	return ((u16_union) hostshort).u8[1] | ((u16_union) hostshort).u8[0] << 8;
}