#include "LED.h"

#include "em_device.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "em_dma.h"
#include "em_int.h"
#include "em_cmu.h"

#define CLK_PORT 4
#define CLK_PIN 5
#define MOSI_PORT 4
#define MOSI_PIN 7

#define USART USART0

static uint8_t LEDS[NUMLEDS+STOPBYTES];
static uint8_t oldCol[8*3];

DMA_CB_TypeDef LEDcallback = {.cbFunc = LED_dmaDone,
							.userPtr = 0,
							.primary = 0
};

static uint8_t dmaDone = 1;
static uint8_t updateWaiting = 0;

LED_color_t row1Color = {
		.r = 0x0,
		.g = 0x0,
		.b = 0xFF
};
LED_color_t row2Color = {
		.r = 0x0,
		.g = 0x0,
		.b = 0xFF
};
LED_color_t row3Color = {
		.r = 0x0,
		.g = 0x0,
		.b = 0xFF
};
LED_color_t row4Color = {
		.r = 0x0,
		.g = 0xFF,
		.b = 0x00
};
LED_color_t row5Color = {
		.r = 0x0,
		.g = 0xFF,
		.b = 0x00
};
LED_color_t row6Color = {
		.r = 0xFF,
		.g = 0x0,
		.b = 0x00
};
LED_color_t row7Color = {
		.r = 0xFF,
		.g = 0x0,
		.b = 0x00
};
LED_color_t row8Color = {
		.r = 0xFF,
		.g = 0x0,
		.b = 0x00
};
LED_color_t off = {
	.r = 0,
	.g = 0,
	.b = 0
};


void LED_dmaDone(unsigned int channel, bool primary, void *user){
	dmaDone = 1;
	USART->CTRL &= ~USART_CTRL_CSINV;
	USART->CMD |= USART_CMD_TXDIS;
}

void LED_init(void){
	CMU_ClockEnable(cmuClock_USART0, 1);
	CMU_ClockEnable(cmuClock_GPIO, 1);

	GPIO_PinModeSet(CLK_PORT, CLK_PIN, gpioModePushPull, 1);
	GPIO_PinModeSet(MOSI_PORT, MOSI_PIN, gpioModePushPull, 1);

	USART->FRAME |= USART_FRAME_DATABITS_EIGHT;
	USART->CLKDIV |= 6144 << _USART_CLKDIV_DIV_SHIFT;
	USART->CTRL |= USART_CTRL_AUTOTRI |
			  USART_CTRL_SYNC |
			  USART_CTRL_MSBF;
	USART->CMD = USART_CMD_CLEARRX |
					 USART_CMD_CLEARTX |
					 USART_CMD_MASTEREN;
	USART->ROUTE = USART_ROUTE_LOCATION_LOC1 |
				   USART_ROUTE_CLKPEN |
				   USART_ROUTE_TXPEN;

	DMA_CfgChannel_TypeDef dmaChanInit = {
			.highPri = 0,
			.enableInt = 1,
			.select = DMA_CH_CTRL_SIGSEL_USART0TXEMPTY | DMA_CH_CTRL_SOURCESEL_USART0,
			.cb = &LEDcallback
	};

	DMA_CfgChannel(1, &dmaChanInit);

	DMA_CfgDescr_TypeDef dmaCfgInit = {
			.dstInc = dmaDataIncNone,
			.srcInc = dmaDataInc1,
			.size = dmaDataSize1,
			.arbRate = dmaArbitrate1
	};

	DMA_CfgDescr(1, 1, &dmaCfgInit);

	LED_clear();
	LED_run();
	volatile int i;
	for(i = 0; i < 100000; i++);

}

void LED_run(void){
	INT_Disable();
	if (dmaDone && updateWaiting){
		USART->CTRL |= USART_CTRL_CSINV;
		USART->CMD |= USART_CMD_TXEN;
		DMA_ActivateBasic(1, 1, 0, &(USART->TXDATA), LEDS, NUMLEDS+STOPBYTES-1);
		DMA_ChannelEnable(1, 1);
		dmaDone = 0;
		updateWaiting = 0;
	}
	INT_Enable();
}

void LED_set(uint8_t x, uint8_t y, LED_color_t* color){
	uint32_t ledNo;
	if (!(y%2)){
		ledNo = (x*2+WIDTH*y)*3;
	}
	else{
		ledNo = (16-((x+1)*2)+WIDTH*y)*3;
	}
	LEDS[ledNo] = color->b | 0x80;
	LEDS[ledNo+1] = color->r | 0x80;
	LEDS[ledNo+2] = color->g | 0x80;
	LEDS[ledNo+3] = color->b | 0x80;
	LEDS[ledNo+4] = color->r | 0x80;
	LEDS[ledNo+5] = color->g | 0x80;
	updateWaiting = 1;
}

void LED_turnOn(uint8_t x, uint8_t y){
	switch(y){
	case 0:
		LED_set(x, y, &row1Color);
		break;
	case 1:
		LED_set(x, y, &row2Color);
		break;
	case 2:
		LED_set(x, y, &row3Color);
		break;
	case 3:
		LED_set(x, y, &row4Color);
		break;
	case 4:
		LED_set(x, y, &row5Color);
		break;
	case 5:
		LED_set(x, y, &row6Color);
		break;
	case 6:
		LED_set(x, y, &row7Color);
		break;
	case 7:
		LED_set(x, y, &row8Color);
		break;
	default:
		break;
	}
}

void LED_turnOff(uint8_t x, uint8_t y){
	LED_set(x, y, &off);
}

void LED_clear(void){
	int i;
	for(i = 0; i<NUMLEDS; i++){
		LEDS[i] = 0x80;
	}
	for(i = NUMLEDS; i < NUMLEDS+STOPBYTES; i++){
		LEDS[i] = 0;
	}
	updateWaiting = 1;
}

void setWhiteCol(uint8_t col){
	uint8_t y;
	for(y = 0; y<8; y++){
		uint32_t ledNo;
		uint8_t oldColNo = col-1;
		if(oldColNo > 7){
			oldColNo = 7;
		}
		if (!(y%2)){
			ledNo = ((oldColNo)*2+WIDTH*y)*3;
		}
		else{
			ledNo = (16-((oldColNo)*2)+WIDTH*y)*3;
		}
			LEDS[ledNo] = oldCol[oldColNo*3];
			LEDS[ledNo+1] = oldCol[oldColNo*3+1];
			LEDS[ledNo+2] = oldCol[oldColNo*3+2];
			LEDS[ledNo+3] = oldCol[oldColNo*3];
			LEDS[ledNo+4] = oldCol[oldColNo*3+1];
			LEDS[ledNo+5] = oldCol[oldColNo*3+2];
		if (!(y%2)){
			ledNo = (col*2+WIDTH*y)*3;
		}
		else{
			ledNo = (16-((col+1)*2)+WIDTH*y)*3;
		}
		oldCol[col*3] = LEDS[ledNo];
		oldCol[col*3+1] = LEDS[ledNo+1];
		oldCol[col*3+2] = LEDS[ledNo+2];
		LEDS[ledNo] = LEDS[ledNo] & 0x80;
		LEDS[ledNo+1] = LEDS[ledNo+1] & 0x80;
		LEDS[ledNo+2] = LEDS[ledNo+2] & 0x80;
		LEDS[ledNo+3] = LEDS[ledNo+3] & 0x80;
		LEDS[ledNo+4] = LEDS[ledNo+4] & 0x80;
		LEDS[ledNo+5] = LEDS[ledNo+5] & 0x80;
		updateWaiting = 1;
		}
}

