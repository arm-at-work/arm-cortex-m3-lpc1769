/*
 ===============================================================================
 Name        : systick-interrupt-delay.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-03-02
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

/* protótipos */
void sysTickConfig(void); // configura systick
void delayUs(unsigned int); // implementa delay em microssegundos
void ledConfig(void); // configura pino de led

/* isr do systick */
void SysTick_Handler(void) {
	/* apesar de não estar implementando qualquer alteração no código,
	 caso não seja declarada, o delay não funcionará. */
}

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 100mhz, pclk = 25mhz, etc
	SystemCoreClockUpdate(); // atualiza valor de clock da cpu (c_clk), para a variável SystemCoreClock
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
	SysTick->LOAD |= (SystemCoreClock / 1000000) - 1; // valor de reinício para o contador
	SysTick->CTRL |= (1 << 1 | 1 << 2); // habilita interrupções; fonte de decremento é c_clk
	SCB->SHP[11] |= (0 << 5 | 1 << 3); // nível 0 e subnível 1 de prioridade de interrupção
}

/* implementa delay em microssegundos */
void delayUs(unsigned int us) {
	static unsigned int counter; // contador de interrupções
	SysTick->CTRL |= (1 << 0); // inicializa contador
	for (counter = 0; counter < us; counter++) { // aguarda até que valor do contador se iguale ao valor recebido via parâmetro
		do { // aguarda até que flag de interrupção seja setada - uma chamada a qualquer um dos bits de CTRL, já limpa a flag de interrupção
		} while (!(SysTick->CTRL & (1 << 16)));
	}
	SysTick->CTRL &= ~(1 << 0); // para contador
}

/* configura pino de led */
void ledConfig(void) {
	/* pino p0.22 */
	LPC_PINCON->PINSEL1 &= ~(0b11 << 12); // pino como gpio
	LPC_GPIO0->FIODIR |= (1 << 22); // pino como saída
	LPC_GPIO0->FIOPIN &= ~(1 << 22); // estado lógico baixo
}
