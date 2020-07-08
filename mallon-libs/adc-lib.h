/*
 ===============================================================================
 Name        : uart3-lib.h
 Author      : Thiag Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-04-10
 Description : main definition
 ===============================================================================
 */

#ifndef ADC_LIB_H_
#define ADC_LIB_H_

void adcInit(void) {
	/* configurações gerais */
	LPC_SC->PCONP |= (1 << 12); // habilita periférico adc
	LPC_SC->PCLKSEL0 &= ~(0b11 << 24); // pclk_adc = c_clk / 4
	LPC_PINCON->PINSEL3 |= (0b11 << 30); // pino como AD0.5
	LPC_PINCON->PINMODE3 = ((LPC_PINCON->PINMODE3 & ~(0b11 << 30))
			| (0b10 << 30)); // desabilita pull-up e pull-down do pino
	/* configurações de operação */
	LPC_ADC->ADCR |= (1 << 5 | 1 << 8 | 1 << 16 | 1 << 21); // habilita conversão no canal 5; clkdiv = 1 (pclk_adc / 2); modo burst; adc em operação
	/* configurações de interrupção */
	LPC_ADC->ADINTEN |= (1 << 5); // habilita interrupção por conversão completada no canal 5
	NVIC->IP[22] |= (0 << 5 | 2 << 3); // nível 0 e subnível 2 de prioridade de interrupção
	NVIC->ISER[0] |= (1 << 22); // habilita interrupções no adc
	NVIC->ICPR[0] |= (1 << 22); // retira qualquer pendência de interrupção do adc
}

#endif /* ADC_LIB_H_ */
