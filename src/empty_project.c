/**************************************************************************//**
 * @file
 * @brief Empty Project
 * @author Energy Micro AS
 * @version 3.20.2
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silicon Labs Software License Agreement. See 
 * "http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt"  
 * for details. Before using this software for any purpose, you must agree to the 
 * terms of that agreement.
 *
 ******************************************************************************/
#include "em_device.h"
#include "em_chip.h"
#include "em_dma.h"
#include "em_cmu.h"
#include "stdint.h"

#include "LED.h"

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/

DMA_DESCRIPTOR_TypeDef dmaDesc[12*2] __attribute__ ((aligned(256)));

LED_color_t blue = {.b = 0x7F};
LED_color_t green = {.g = 0x7F};
LED_color_t red = {.r = 0x7F};
LED_color_t white = {.r = 0x7f, .g = 0x74, .b = 0x7f};
LED_color_t nothing = {.r = 0x00, .g = 0x00, .b = 0x00};

uint8_t active[8][8];

uint16_t msTillResend = 550;
uint32_t msToPlay = 500;
uint8_t currentCol = 0;

void TIMER0_IRQHandler(void){
	TIMER0->CMD = TIMER_CMD_STOP;
	LED_set(currentCol, 7,  &nothing);
	currentCol = (currentCol+1)%8;
	RPI_send(&(active[(currentCol+2)%8][0]), 8, msToPlay);
	TIMER0->IFC |= TIMER_IFC_OF;
	LED_set(currentCol, 7,  &white);
}

int main(void)
{
  /* Chip errata */
  CHIP_Init();

  CMU_OscillatorEnable(cmuOsc_HFXO, true, true);
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);

  CMU_ClockEnable(cmuClock_DMA, 1);
  DMA_Init_TypeDef dmaInit = {.hprot = 0,
  	  	  	  	  	  	  	  .controlBlock = dmaDesc
  };

 CMU_ClockEnable(cmuClock_TIMER0, 1);
 TIMER0->CTRL |= TIMER_CTRL_PRESC_DIV1024;
 TIMER0->IEN |= TIMER_IEN_OF;
 NVIC_EnableIRQ(TIMER0_IRQn);
 TIMER0->TOP =	48*msTillResend;

 DMA_Init(&dmaInit);
 LED_init();
 LED_clear();
 LED_run();
 volatile int i;
 for(i = 0; i<1000000; i++);
 LED_clear();
 LED_run();
 RPI_init();
 BUT_init();
 RPI_send(&(active[currentCol][0]), 8, msToPlay);


 uint8_t updated;
 while(true){
	 updated = BUT_updateActive(active);
	 if (updated){
		 LED_run();
	 }
 }

//  uint8_t col[8] = {1, 2, 3, 4, 5, 6, 7, 8};
//  RPI_send(col, 8, 123);
//
//  while(1);
//
//  /* Infinite loop */
//  int led = 0;
//  while (1) {
//	  if (led > 2){
//		  LED_set(led-3, 0, &nothing);
//		  LED_set(led-2, 0, &nothing);
//		  LED_set(led-1, 0, &nothing);
//	  }
//	  LED_set(led, 0, &white);
//	  LED_set(led+1, 0, &white);
//	  LED_set(led+2, 0, &white);
//	  LED_run();
//	  volatile int i;
//	  for(i = 0; i < 100000; i++);
//	  led += 1;
//	  if(led > 64){
//		  LED_clear();
//		  led = 0;
//	  }
//  }
}
