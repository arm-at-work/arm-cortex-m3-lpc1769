/*
 ===============================================================================
 Name        : pull-up-w-interrupt-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-02-21
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>
#include "../../../mallon-libs/uart3-lib.h"

unsigned int sysTickCount = 0; // contador para interrupção de SysTick

void buttonsConfig(void);
void uartInit(void);
void sysTickConfig();
void delayUs(unsigned int microseconds);

/** ISR do SysTick */
void SysTick_Handler(void) {
	sysTickCount++; // incrementa contador do SysTick
}

int main(void) {
	SystemInit(); // função CMSIS - configura sistema: C_CLK = 100MHz, PCLK = 25MHz e etc.
	SCB->AIRCR |= 0x05FA0000 | 0b100 << 8; // prigroup = b100 - 8 nível e 4 subníveis de prioridade
	sysTickConfig(); // função configura SysTick
	buttonsConfig(); // configura botão
	uart3_rs232_init(); // chama função que inicializa uart3
	do { // loop infinito
		if (!(LPC_GPIO0->FIOPIN & (1 << 4))) { // verifica se botão foi pressionado - se pino está com nível lógico 0
			printf("%s\n", "botão pressionado!");
			delayUs(77000); // implementa intervalo de 77ms
		}
	} while (1);

	return 1;
}

/** função configura pino para ser utilizado como botão, ligando os resistores de pull-up */
void buttonsConfig(void) {
	LPC_PINCON->PINSEL0 &= ~(0b11 << 8); // configura pino como GPIO
	LPC_GPIO0->FIODIR &= ~(1 << 4); // configura pino como saída
	LPC_PINCON->PINMODE0 &= ~(0b11 << 4); // habilita pullup no pino
	LPC_GPIO0->FIOPIN |= (1 << 4); // atribui nível lógico alto ao pino
}

/** função configura o SysTick */
void sysTickConfig(void) {
	SystemCoreClockUpdate(); // atualiza valor de frequência do clock do sistema
	SysTick->LOAD = (SystemCoreClock / 1000000) - 1; // define período de reinício para contador do SysTick
	SCB->SHP[11] = 0 << 4 | 0 << 3; // nível 0 e subnível 0 de prioridade de interrupção
	SysTick->CTRL = 0 << 0 | 1 << 1 | 1 << 2; // não habilita contador; habilita interrupção para SysTick; fonte de clock é C_CLK
}

/** função implementa intervalo em microssegundos, via SysTick */
void delayUs(unsigned int microseconds) {
	sysTickCount = 0; // zera contador do SysTick
	SysTick->CTRL |= 1 << 0; //Habilita o contador.

	do { //Aguarda até que o número de interrupções seja alcançado.
	} while (sysTickCount < microseconds);

	SysTick->CTRL &= ~(1 << 0); //Desabilita o contador.
}
