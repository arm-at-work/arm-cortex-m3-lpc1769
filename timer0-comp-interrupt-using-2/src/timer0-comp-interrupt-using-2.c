/*
 ===============================================================================
 Name        : timer0-using.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-02-08
 Description : main definition

 Neste exemplo o canal MR0 tem valor inicial configurado, para que seja gerada interrupção após 1s.
 Na interrupção incrementamos o valor de MR0, para que seja gerada mais uma interrupção 1s depois
 e assim por diante, até estouro do contador do timer0.
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

void gpio_config(void); // configura pinos como MAT
void timer0_config(void); // configura timer0

void TIMER0_IRQHandler(void) {
	if (LPC_TIM0->IR & (1 << 0)) { // verifica se flag de interrupção por comparação do canal 0 do timer0 está setada
		LPC_TIM0->MR0 += 12499; // altera valor de registrador de comparação
		LPC_GPIO1->FIOPIN ^= (0b11 << 28);
		LPC_TIM0->IR |= (1 << 0); // limpa flag de interrupção por comparação no canal 0 do timer0
	}
}

int main(void) {
	SystemInit(); // função CMSIS inicializa o sistema. C_CLK = 100MHz e 25MHz para periféricos
	gpio_config(); // configura pinos como MAT
	timer0_config(); // configura timer0
	SCB->AIRCR = 0x05FA0000 | 0b100 << 8; // prigroup b100 (8 níveis e 4 subníveis de prioridade)
	do { // loop infinito
	} while (1);

	return 1;
}

/** configura pinos como MAT */
void gpio_config(void) {
	LPC_PINCON->PINSEL3 &= ~(0b11 << 24); // configura pinos como GPIO
	LPC_GPIO1->FIODIR |= (1 << 28); // pinos como saída
	LPC_GPIO1->FIOPIN &= ~(1 << 28); // pinos com estado lógico 0
}

/** configura timer0 */
void timer0_config(void) {
	LPC_SC->PCONP |= (1 << 1); // liga timer0
	LPC_SC->PCLKSEL0 &= ~(0b11 << 1); // seleciona clock de C_CLK / 4 (25MHz) para timer0
	LPC_TIM0->CTCR &= ~(0b11 << 1); // fonte de incremento do timer0 é borda da subida de PCLK_TIMER0
	LPC_TIM0->PR = 2000; // seta prescaler
	LPC_TIM0->MR0 = 12499; // seta valor de comparação
	LPC_TIM0->MCR &= ~(0b11 << 1 | 0b11 << 4 | 0b11 << 7 | 0b11 << 10); // nada ocorre em coincidências de comparados nos canais 0, 1 e 2
	LPC_TIM0->MCR |= (1 << 0); // dispara interrução por coincidência na comparação do canal 0
	LPC_TIM0->EMR &= ~(0b11 << 4 | 0b11 << 6); // após coincidência de comparador, não ocorre ação nos pinos MAT
	LPC_TIM0->TCR = 1; // inicializa contador do timer0
	NVIC->IP[1] |= (0 << 5 | 0 << 3); // prioridade de interrupção nível 0 e subnível 0
	NVIC->ISER[0] |= (1 << 1); // habilita interrupções para timer0
	NVIC->ICPR[0] |= (1 << 1); // retira qualquer interrupção pendente no timer0
}
