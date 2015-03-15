/*
 ===============================================================================
 Name        : adc-using.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-03-07
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>
#include "../../mallon-libs/uart3-lib.h" // biblioteca particular de uso da UART3

void adcInit(void); // função inicializa módulo ADC

/** rotina de interrupção */
void ADC_IRQHandler(void) {
	uint8_t channel = ((LPC_ADC->ADGDR & (0xFFF << 24)) >> 24); // captura canal
	if (!(LPC_ADC->ADSTAT & (channel & (1 << 8)))) { // verifica se ocorreu overun no dado convertido do canal
		printf("%u\n", ((LPC_ADC->ADGDR & (0xFFF << 4)) >> 4)); // imprime valor convertido - a simples operação de leitura do valor, já limpa a flag de interrupção do canal
		LPC_ADC->ADCR |= 0b001 << 24; // solicita nova conversão
	}
}

int main(void) {
	SystemInit(); // inicializa sistema
	SCB->AIRCR = 0x05FA0000 | 0b100 << 8; // seleciona prigroup 100 (8 nível e 4 subníveis de prioridade de interrupção)
	uart3_rs232_init(); // inicializa UART3
	adcInit(); // função inicializa o módulo ADC
	do { // loop infinito
	} while (1);
	return 1;
}

/** função inicializa módulo ADC */
void adcInit(void) {
	/* configurações gerais */
	LPC_SC->PCONP |= (1 << 12); // liga ADC
	LPC_SC->PCLKSEL0 &= ~(0b11 << 24); // atribui clock de C_CLK / 4
	/* configurações de pinos */
	LPC_PINCON->PINSEL3 |= (0b11 << 30); // configura pino como AD0.5
	LPC_PINCON->PINMODE3 = ((LPC_PINCON->PINMODE3 & ~(0b11 << 30))
			| (0b10 << 30)); // desabilita resistores de pull-up e pull-down do pino
	LPC_ADC->ADCR |= 1 << 5 | 0 << 16 | 1 << 21 | 0b001 << 24; // seleciona canal 5 para conversão; modo simples; ADC em operação; realiza conversão
	LPC_ADC->ADCR &= ~(0b1111 << 8); // CLKDIV = 0
	/* configurações de interrupção */
	LPC_ADC->ADINTEN |= 1 << 5; // habilita interrupções por conversão completada no canal 5 do ADC
	NVIC->IP[22] |= (1 << 5 | 1 << 3); // atribui nível 0 e subnível 0 de prioridade de interrupção (nível mais alto possível)
	NVIC->ISER[0] |= (1 << 22); // habilita interrupções no ADC
	NVIC->ICPR[0] |= (1 << 22); // retira qualquer interrupção pendente do ADC
}
