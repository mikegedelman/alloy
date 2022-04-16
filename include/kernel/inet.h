#ifndef _KERNEL_INET_H
#define _KERNEL_INET_H

#include <stdint.h>

// uint32_t htonl(uint32_t hostlong) {
// 	u_char *s = (u_char *)&x;
//     return (uint32_t)(s[0] << 24 | s[1] << 16 | s[2] << 8 | s[3]);
// }


uint16_t htons(uint16_t hostshort);
uint16_t ntohs(uint16_t netshort);

#endif
