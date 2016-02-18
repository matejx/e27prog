#ifndef MAT_E27_H
#define MAT_E27_H

#include <inttypes.h>

void e27_vpp(uint8_t on);
void e27_init(void);
uint8_t e27_rd(uint16_t a);
uint8_t e27_wr(uint16_t a, uint8_t d);

#endif
