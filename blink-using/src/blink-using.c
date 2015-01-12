/*
 ===============================================================================
 Name        : blink-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-01-07
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

unsigned int sysTickCount = 0; // contador para interrupção de SysTick

void sysTickConfig(); // configura SysTick
void delayUs(unsigned int microseconds); // implementa intervalo segundo o parâmetro passado

void SysTick_Handler(void) {
	sysTickCount++; // incrementa contador
}

int main(void) {
	SystemInit(); // função CMSIS - configura sistema: C_CLK = 100MHz, PCLK = 25MHz e etc.
	SCB->AIRCR |= 0x05FA0000 | 0b100 << 8; // prigroup = b100 - 8 nível e 4 subníveis de prioridade
	sysTickConfig(); // função configura SysTick

	LPC_PINCON->PINSEL3 &= ~(0b11 << 24 | 0b11 << 26);
	LPC_PINCON->PINSEL7 &= ~(0b11 << 18 | 0b11 << 20); // configura pinos como GPIO

	LPC_GPIO1->FIODIR |= (1 << 28 | 1 << 29);
	LPC_GPIO3->FIODIR |= (1 << 25 | 1 << 26); // configura pinos GPIO como saída

	while (1) {
		LPC_GPIO1->FIOPIN ^= (1 << 28 | 1 << 29);
		LPC_GPIO3->FIOPIN ^= (1 << 25 | 1 << 26); // inverte nível lógico dos pinos

		delayUs(20000); // valor 1000000 implementa delay de 1s; valor 20000 implementa delay de 20ms
	}
	return 0;
}

void sysTickConfig(void) {
	SystemCoreClockUpdate(); // atualiza valor de frequência do clock do sistema

	SysTick->LOAD = (SystemCoreClock / 1000000) - 1; // define período de reinício para contador do SysTick
	SCB->SHP[11] = 0 << 4 | 0 << 3; // nível 0 e subnível 0 de prioridade de interrupção
	SysTick->CTRL = 0 << 0 | 1 << 1 | 1 << 2; // não habilita contador; habilita interrupção para SysTick; fonte de clock é C_CLK
}

void delayUs(unsigned int microseconds) {
	sysTickCount = 0;
	SysTick->CTRL |= 1 << 0; // habilita o contador.

	do { //Aguarda até que o número de interrupções seja alcançado.
	} while (sysTickCount < microseconds);

	SysTick->CTRL &= ~(1 << 0); // desabilita o contador.
}
