#include "but.h"

#include "em_chip.h"
#include "em_gpio.h"
#include "led.h"

static uint8_t pushedBut[8][8]; //col, row

void BUT_init(void){
	int i;
	for(i = 0; i < 8; i++){
		GPIO_PinModeSet(3, i, gpioModePushPull, 0);
	}
	for(i = 0; i < 8; i++){
			GPIO_PinModeSet(0, i, gpioModeInputPullFilter, 0);
	}
}

uint8_t BUT_updateActive(uint8_t active[8][8]){
	uint8_t in = 0;
	int j; //row
	int i; //col
	volatile int kuk;
	uint8_t on;
	uint8_t retval = 0;
	for(i = 0; i<8; i++){
		GPIO->P[3].DOUTSET = 1 << i;
		for(kuk = 0; kuk < 10000; kuk++);
		in = (uint8_t)(GPIO->P[0].DIN & 0xff);
		for(j = 0; j<8; j++){
			on = (in>>j)&0x01;
			if(pushedBut[j][i] != on){
				retval = 1;
				if(on && !active[j][i]){
					LED_turnOn(j, i);
					active[j][i] = 1;
				}
				else if(on && active[j][i]){
					LED_turnOff(j, i);
					active[j][i] = 0;
				}
				pushedBut[i][j] = on;
			}
		}
		GPIO->P[3].DOUTCLR = 1 << i;
	}
	return retval;

}
