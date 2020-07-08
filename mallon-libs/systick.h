/*
 ===============================================================================
 Name        : uart3-lib.h
 Author      : Thiag Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-04-02
 Description : main definition
 ===============================================================================
 */
#ifndef SYSTICK_H_
#define SYSTICK_H_

#include "lpc17xx.h"

void sysTickConfig(void);
void delayUs(uint32_t);

void sysTickConfig(void) {
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1;
	SysTick->CTRL |= (1 << 2);
}

void delayUs(uint32_t us) {
	static uint32_t count;
	SysTick->CTRL |= (1 << 0);
	for (count = 0; count < us; count++) {
		do {
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0);
}

#endif /* SYSTICK_H_ */
