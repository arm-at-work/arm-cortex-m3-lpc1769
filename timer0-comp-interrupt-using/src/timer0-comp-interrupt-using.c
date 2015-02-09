/*
 ===============================================================================
 Name        : timer0-using.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at	 : 2015-02-06
 Description : main definition

 Neste exemplo os canais de comparação tem valor de 1s acima do outro, ou seja, o canal MR0
 tem valor para que seja gerada uma interrupção 1s após inicialização do contador do timer,
 o MR2 tem valor para que seja gerada interrupção 2s depois da inicialização do contador do timer,
 e assim por diante, depois de ocorrer a interrupção no canal 3, o contador é resetado, por instrução
 de software.
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

void gpio_config(void); // configura pinos GPIO
void timer0_config(void); // configura timer0

void TIMER0_IRQHandler(void) {
	if (LPC_TIM0->IR & (1 << 0)) { // verifica se interrupção ocorreu no canal 0 de comparação
		LPC_GPIO1->FIOPIN = ((LPC_GPIO1->FIOPIN & ~(1 << 28)) | (1 << 29));
		LPC_GPIO3->FIOPIN &= ~(0b11 << 25);
		LPC_TIM0->IR |= (1 << 0); // limpa flag de interrupção por comparação no canal 0
	} else if (LPC_TIM0->IR & (1 << 1)) { // verifica se interrupção ocorreu no canal 1 de comparação
		LPC_GPIO1->FIOPIN = ((LPC_GPIO1->FIOPIN & ~(1 << 29)) | (1 << 28));
		LPC_GPIO3->FIOPIN &= ~(0b11 << 25);
		LPC_TIM0->IR |= (1 << 1); // limpa flag de interrupção por comparação no canal 0
	} else if (LPC_TIM0->IR & (1 << 2)) { // verifica se interrupção ocorreu no canal 2 de comparação
		LPC_GPIO3->FIOPIN = ((LPC_GPIO3->FIOPIN & ~(1 << 25)) | (1 << 26));
		LPC_GPIO1->FIOPIN &= ~(0b11 << 28);
		LPC_TIM0->IR |= (1 << 2); // limpa flag de interrupção por comparação no canal 0
	} else if (LPC_TIM0->IR & (1 << 3)) { // verifica se interrupção ocorreu no canal 3 de comparação
		LPC_GPIO3->FIOPIN = ((LPC_GPIO3->FIOPIN & ~(1 << 26)) | (1 << 25));
		LPC_GPIO1->FIOPIN &= ~(0b11 << 28);
		LPC_TIM0->IR |= (1 << 3); // limpa flag de interrupção por comparação no canal 0
	}
}

int main(void) {
	SystemInit(); // função CMSIS inicializa o sistema. C_CLK = 100MHz e 25MHz para periféricos
	gpio_config(); // configura pinos GPIO
	timer0_config(); // configura timer0
	SCB->AIRCR = 0x05FA0000 | 0b100 << 8; // PRIGROUP = b100 (8 níveis e 4 subníveis)
	do { // loop infinito
	} while (1);

	return 1;
}

/** configura pinos GPIO */
void gpio_config(void) {
	LPC_PINCON->PINSEL3 &= ~(0b11 << 24 | 0b11 << 26); // configura pinos como GPIO
	LPC_GPIO1->FIODIR |= (1 << 28 | 1 << 29); // pinos como saída
	LPC_GPIO1->FIOPIN &= ~(1 << 28 | 1 << 29); // pinos com estado lógico 0

	LPC_PINCON->PINSEL7 &= ~(0b11 << 18 | 0b11 << 20); // configura pinos como GPIO
	LPC_GPIO3->FIODIR |= (1 << 25 | 1 << 26); // pinos como saída
	LPC_GPIO3->FIOPIN &= ~(1 << 25 | 1 << 26); // pinos com estado lógico 0
}

/** configura timer0 */
void timer0_config(void) {
	LPC_SC->PCONP |= (1 << 1); // liga timer0
	LPC_SC->PCLKSEL0 &= ~(0b11 << 1); // seleciona clock de C_CLK / 4 (25MHz) para timer0
	LPC_TIM0->CTCR &= ~(0b11 << 1); // fonte de incremento do timer0 é borda da subida de PCLK_TIMER0
	LPC_TIM0->PR = 2000; // seta prescaler
	LPC_TIM0->MR0 = 12499; // seta valor de comparação
	LPC_TIM0->MR1 = 24999; // seta valor de comparação
	LPC_TIM0->MR2 = 37498; // seta valor de comparação
	LPC_TIM0->MR3 = 49997; // seta valor de comparação
	LPC_TIM0->MCR &= ~(0b11 << 1 | 0b11 << 4 | 0b11 << 7); // nada ocorre em coincidências de comparados nos canais 0, 1 e 2
	LPC_TIM0->MCR = ((LPC_TIM0->MCR & ~(0b11 << 10)) | (0b01 << 10)); // quando ocorrer coincidência de comparador no canal 3, contador é reiniciado
	LPC_TIM0->MCR |= (1 << 0 | 1 << 3 | 1 << 6 | 1 << 9); // dispara interrução na coincidência de qualquer dos canais
	LPC_TIM0->EMR &= ~(0b11 << 4 | 0b11 << 6); // após coincidência de comparador, não ocorre ação nos pinos MAT
	LPC_TIM0->TCR = 1; // inicializa contador do timer0
	NVIC->IP[1] |= (0 << 5 | 0 << 3); // interrupções do timer0 com nível 0 e subnível 0
	NVIC->ISER[0] |= (1 << 1); // habilita interrupções para timer0
	NVIC->ICPR[0] |= (1 << 1); // limpa interrupções pendentes do timer0
}
