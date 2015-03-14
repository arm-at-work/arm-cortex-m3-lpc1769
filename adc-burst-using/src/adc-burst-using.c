/*
 ===============================================================================
 Name        : adc-burst-using.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-03-12
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>
#include "../../mallon-libs/uart3-lib.h"

void adcInit(void);  // configura/inicializa adc

/* isr do adc */
void ADC_IRQHandler(void) {
	if (LPC_ADC->ADSTAT & (1 << 16)) {
		printf("%s%u\n", "Valor convertido: ",
				((LPC_ADC->ADGDR & (0xFFF << 4)) >> 4));
	}
}

int main(void) {
	SystemInit(); // inicializa o sistema - c_clk = 100mhz, pclk = 25mhz
	SCB->AIRCR |= 0x05FA0000 | 0b100 << 8; // prigroup 0b100; 8 níveis e 4 subníveis de prioridade
	uart3_rs232_init(); // configura/inicializa uart3 ( necessário que venha antes do adcInit(), senão não funciona)
	adcInit(); // configura/inicializa adc
	do { // loop infinito
	} while (1);
	return 1;
}

/** configura/inicializa adc */
void adcInit(void) {
	/** configurações gerais */
	LPC_SC->PCONP |= (1 << 12); // habilita periférico adc
	LPC_SC->PCLKSEL0 |= (0b11 << 24); // pclk_adc = c_clk / 4
	LPC_PINCON->PINSEL3 |= (0b11 << 30); // pino como AD0.5
	LPC_PINCON->PINMODE3 = ((LPC_PINCON->PINMODE3 & ~(0b11 << 30))
			| (0b10 << 30)); // desabilia pull-up e pull-down do pino
	/** configurações de operação */
	LPC_ADC->ADCR |= (1 << 5 | 1 << 16 | 1 << 8 | 1 << 21);
	/** configurações de interrupção */
	LPC_ADC->ADINTEN |= (0b111 << 8); // habilita interrupção por conversão completada no canal 5
	NVIC->IP[22] |= (0 << 5 | 0 << 3); // nível 0 e subnível 0 de prioridade de interrupção
	NVIC->ISER[0] |= (1 << 22); // habilita interrupções no adc
	NVIC->ICPR[0] |= (1 << 22); // retira qualquer interrupção pendente no adc
}

