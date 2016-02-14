#ifndef RPICOMM_H
#define RPICOMM_H

#include "stdbool.h"
#include "stdint.h"

void RPI_init(void);

void RPI_dmaDone(unsigned int channel, bool primary, void *user);

void RPI_send(uint8_t* col, uint8_t colLen, uint32_t lenMs);

uint8_t RPI_ready(void);

#endif //RPICOMM_H
