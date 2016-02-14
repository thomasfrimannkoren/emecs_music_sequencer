#ifndef LED_H
#define LED_H

#include "stdbool.h"
#include "stdint.h"

#define NUMCOLORS 3
#define STOPBYTES ((WIDTH*HEIGHT)+1)/32 + 1
#define WIDTH 16
#define HEIGHT 8
#define NUMLEDS (WIDTH*HEIGHT)*NUMCOLORS


typedef struct LED_color_t{
	uint8_t b;
	uint8_t r;
	uint8_t g;
}LED_color_t;

void LED_dmaDone(unsigned int channel, bool primary, void *user);
void LED_init(void);
void LED_run(void);
void LED_set(uint8_t x, uint8_t y, LED_color_t* color);
void LED_turnOn(uint8_t x, uint8_t y);
void LED_turnOff(uint8_t x, uint8_t y);
void LED_clear(void);
void setWhiteCol(uint8_t col);

#endif
