/*
 ===============================================================================
 Name        : timer0-using.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-02-04
 Description : main definition

 Cálculo para valor de TIMER0PR:
 Primeiro calcula-se a frequência com base no tempo de delay desejado (desejamos delay de 20ms): Freq_alvo = ((1 / 20ms) * 100) = 50Hz
 Com base na valor de frequência alvo, calculamos o valor para TIMER0MR (o valor para TIMER0PR (prescaler) é escolhido por nós mesmo,
 mas eu geralmente coloco um valor de cálculo (PR + 1) igual ao da frequência encontrado, a não ser que ele gere um número fracionado,
 aí coloco algum valor próximo, que não gere fração:
 TIMER0MR = (((PCLK_TIMER0 / (TIMER0PR + 1)) / Freq_alvo) - 1) ou seja
 9999 = (((25MHz / (49 + 1)) / 50Hz) -1)

 Observe que para o prescaler (TIMER0PR, ou LPC_TIM0->PR no código), colocamos o valor 49,
 para PLCK_TIMER0 (LPC_SC->PCLKSEL0) atribuímos o valor C_CLK / 4 (25MHz)
 e nossa frequência alvo, para o tempo de 20ms, é ((1/20ms) * 1000) = 50Hz
 e com isso obtivemos o resultado 9999 (da subtração do valor 20000 por 1) para o TIMER0MR (LPC_TIM0->MR0),
 que é o valor para comparação.
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

void timer0_mat_config(void); // configura pinos como MAT
void timer0_config(void); // configura timer0

int main(void) {
	SystemInit(); // função CMSIS inicializa o sistema. C_CLK = 100MHz e 25MHz para periféricos
	timer0_mat_config(); // configura pinos como MAT
	timer0_config(); // configura timer0
	do { // loop infinito
	} while (1);

	return 1;
}

/** configura pinos como MAT */
void timer0_mat_config(void) {
	/** configura 1º registros dos MAT[0] e MAT[1] */
	LPC_PINCON->PINSEL3 |= (0b11 << 24 | 0b11 << 26); // configura pinos como MAT[0] e MAT[1] respectivamente

	/** configura 2º registros dos MAT[0] e MAT[1] */
	LPC_PINCON->PINSEL7 |= ((LPC_PINCON->PINSEL7 & ~(0b11 << 18 | 0b11 << 20))
			| (0b10 << 18 | 0b10 << 20)); // configura pinos como MAT[0] e MAT[1] respectivamente
}

/** configura timer0 */
void timer0_config(void) {
	/* configurações gerais */
	LPC_SC->PCONP |= (1 << 1); // bit PCTIM0 - inicializa timer0
	LPC_SC->PCLKSEL0 &= ~(0b11 << 2); // timer0 com clock C_CLK / 4 (25MHz)
	LPC_TIM0->CTCR &= ~(0b11 << 0); // incremento na borda da subida de PLCK_TIMER0 (CTCR - Count Control Register)
	LPC_TIM0->PR = 49; // seta prescaler para o timer
	LPC_TIM0->MR0 = 9999; // (MR - Match Register) valor para intervalo em 20ms. Cálculo: (((25MHz / (49+1) / 50Hz) -1) = 9999 (PR - Prescaler Register)
	LPC_TIM0->MCR |= ((LPC_TIM0->MCR & ~(0b11 << 1)) | (0b01 << 1)); // reinicia o contador principal se ocorrer um evento de comparação no canal 0 (MCR - Match Control Register)
	LPC_TIM0->EMR |= (0b11 << 4 | 0b11 << 6); // inverte estado lógico dos pinos MAT0[0] e MAT[1] de 20ms em 20ms (EMR - External Match Register)
	LPC_TIM0->TCR = 1; // habilita contadores (TCR - Timer Control Register)
}
