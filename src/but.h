#ifndef BUT_H
#define BUT_H

#include "stdbool.h"
#include "stdint.h"

void BUT_init(void);

uint8_t BUT_updateActive(uint8_t active[8][8]);

#endif //BUT_H
