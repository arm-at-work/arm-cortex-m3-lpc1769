/*
 ===============================================================================
 Name        : systick-delay.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-03-20
 Description : Neste exemplo é implementada função de delay em microssegundos, através
 do periférico de processador, SysTick Timer (System Tick Timer). Não é utilizada a
 interrupção no exemplo que se segue, porém, disponibilizo exemplo com mesma função,
 porém, implementada com interrupção.
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

void sysTickConfig(void); // configura systick
void delayUs(unsigned int); // implementa delay em microssegundos
void ledConfig(void); // configura pino de led

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 100mhz, pclk = 25mhz, etc
	SystemCoreClockUpdate(); // atualiza valor de clock da cpu, para a variável SystemCoreClock
	SCB->AIRCR |= 0x05FA0000 | 0b100 << 8; // prigroup b100 - 8 níveis e 4 subníveis de prioridade de interrupção
	sysTickConfig(); // configura systick
	ledConfig(); // configura pino de led
	do { // loop infinito
		delayUs(500000);
		LPC_GPIO0->FIOPIN ^= (1 << 22); // inverte estado lógico do pino
	} while (1);
	return 1;
}

/* configura systick */
void sysTickConfig(void) {
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1; // valor de reinício para o contador
	SysTick->CTRL |= (1 << 2); // fonte de decremento é c_clk
}

/* implementa delay em microssegundos */
void delayUs(unsigned int us) {
	static unsigned int counter; // variável contadora de interrupções
	SysTick->CTRL |= (1 << 0); // habilita contador
	for (counter = 0; counter < us; counter++) { // aguarda até que valor do contador seja igual a valor recebido via parâmetro
		do { // aguarda até que flag seja setada - ocorra evento de interrupção
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0); // para contador
}

/* configura pino de led */
void ledConfig(void) {
	LPC_PINCON->PINSEL1 &= ~(0b11 << 12); // pino como gpio
	LPC_GPIO0->FIODIR |= (1 << 22); // pino como saída
	LPC_GPIO0->FIOPIN &= ~(1 << 22); // estado lógico baixo
}
