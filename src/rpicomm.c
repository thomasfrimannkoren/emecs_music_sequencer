#include "rpicomm.h"

#include "em_device.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "em_dma.h"
#include "em_int.h"
#include "em_cmu.h"
#include "string.h"

#define CLK_PORT 2
#define CLK_PIN 4
#define MISO_PORT 2
#define MISO_PIN 3
#define MOSI_PORT 2
#define MOSI_PIN 2
#define CS_PORT 2
#define CS_PIN 5

#define USART USART2

DMA_CB_TypeDef RPIcallback = {.cbFunc = RPI_dmaDone,
							.userPtr = 0,
							.primary = 0
};

static uint8_t dmaDone = 1;

static uint8_t sendBuffer[20];

void RPI_dmaDone(unsigned int channel, bool primary, void *user){
	dmaDone = 1;
	USART->CMD |= USART_CMD_TXDIS | USART_CMD_RXDIS;
	TIMER0->CMD = TIMER_CMD_START;
}

void RPI_init(void){
#warning Change here
	CMU_ClockEnable(cmuClock_USART2, 1);
	CMU_ClockEnable(cmuClock_GPIO, 1);

	GPIO_PinModeSet(CLK_PORT, CLK_PIN, gpioModeInput, 0);
	GPIO_PinModeSet(MISO_PORT, MISO_PIN, gpioModePushPull, 1);
	GPIO_PinModeSet(MOSI_PORT, MOSI_PIN, gpioModeInput, 0);
	GPIO_PinModeSet(CS_PORT, CS_PIN, gpioModeInput, 0);

	USART->FRAME |= USART_FRAME_DATABITS_EIGHT;
	//USART->CLKDIV |= 6144 << _USART_CLKDIV_DIV_SHIFT;
	USART->CTRL |= USART_CTRL_AUTOTRI |
			  USART_CTRL_SYNC |
			  USART_CTRL_MSBF;
	USART->CMD = USART_CMD_CLEARRX |
					 USART_CMD_CLEARTX |
					 USART_CMD_MASTERDIS;
	USART->ROUTE = USART_ROUTE_LOCATION_LOC0 |
				   USART_ROUTE_CLKPEN |
				   USART_ROUTE_TXPEN |
				   USART_ROUTE_CSPEN |
				   USART_ROUTE_RXPEN;

	DMA_CfgChannel_TypeDef dmaChanInit = {
			.highPri = 0,
			.enableInt = 1,
#warning Change this
			.select = DMA_CH_CTRL_SIGSEL_USART2TXEMPTY | DMA_CH_CTRL_SOURCESEL_USART2,
			.cb = &RPIcallback
	};
	DMA_CfgChannel(2, &dmaChanInit);

	DMA_CfgDescr_TypeDef dmaCfgInit = {
			.dstInc = dmaDataIncNone,
			.srcInc = dmaDataInc1,
			.size = dmaDataSize1,
			.arbRate = dmaArbitrate1
	};
	DMA_CfgDescr(2, 1, &dmaCfgInit);
}

void RPI_send(uint8_t* col, uint8_t colLen, uint32_t lenMs){
	INT_Disable();
	if (dmaDone){
		memcpy(sendBuffer, col, colLen);
		sendBuffer[colLen] = (lenMs & 0xff000000) >> 24;
		sendBuffer[colLen+1] = (lenMs & 0xff0000) >> 16;
		sendBuffer[colLen+2] = (lenMs & 0xff00) >> 8;
		sendBuffer[colLen+3] = lenMs & 0xff;
		USART->CMD |= USART_CMD_TXEN
					| USART_CMD_RXEN;
		DMA_ActivateBasic(2, 1, 0, &(USART->TXDATA), sendBuffer, colLen+4-1);
		DMA_ChannelEnable(2, 1);
		dmaDone = 0;
	}
	INT_Enable();
}

uint8_t RPI_ready(void){
	return dmaDone;
}
