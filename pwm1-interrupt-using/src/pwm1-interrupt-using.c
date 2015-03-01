/*
 ===============================================================================
 Name        : pwm1-interrupt-using.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-02-23
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção ao código da memória flash

/* protótipos */
void pwm1Init(void);

/* isr do pwm1 */
void PWM1_IRQHandler(void) {
	if (LPC_PWM1->IR & (1 << 0)) {
		//LPC_PWM1->MR1 =
		LPC_PWM1->IR |= (1 << 0);
	}
}

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 100mhz, pclk = 25mhz, etc
	SCB->AIRCR |= 0x05FA0000 | 0b100 << 8; // prigroup b100 - 8 níveis e 4 subníveis de prioridade de interrupção
	pwm1Init(); // configura/inicializa pwm1
	do {
	} while (1);
	return 1;
}

/* configura/inicializa pwm1 */
void pwm1Init(void) {
	/* configurações gerais */
	LPC_SC->PCONP |= (1 << 6); // habilita periférico pwm1
	LPC_SC->PCLKSEL0 = ((LPC_SC->PCLKSEL0 & ~(0b11 << 12)) | (0b01 << 12)); // pclk_pwm1 = c_clk
	LPC_PINCON->PINSEL3 = ((LPC_PINCON->PINSEL3 & ~(0b11 << 4)) | (0b10 << 4)); // pino como pwm1[1]
	/* configurações de operação */
	LPC_PWM1->CTCR &= ~(0b11 << 0); // seleciona pclk_pwm1 como fonte de incremento do contador TC
	LPC_PWM1->PR = 1; // atribui valor de prescaler
	LPC_PWM1->MR0 = 999999; // atribui valor de período do pwm1
	LPC_PWM1->MR1 = 500000; // atribui valor de comparação do canal 1
	LPC_PWM1->MCR |= (1 << 1); // reinicía contadores após coincidência de comparação no canal 0
	LPC_PWM1->PCR |= (1 << 9); // habilita saída no pino pwm1[1]
	LPC_PWM1->LER |= (1 << 1); // habilita atualização de valor de comparação do canal 1
	/* configurações de interrupção */
	LPC_PWM1->MCR |= (1 << 0); // habilita interrupção por coincidência de comparação no canal 0
	NVIC->IP[9] |= (0 << 5 | 0 << 3); // nível 0 e subnível 0 de prioridade de interrupção (mais alta prioridade possível)
	NVIC->ISER[0] |= (1 << 9); // habilita interrupções no pwm1
	NVIC->ICPR[0] |= (1 << 9); // retira qualquer pendência de interrupção do pwm1
	/* inicializa pwm1 */
	LPC_PWM1->TCR |= (1 << 0 | 1 << 3); // inicializa pwm1; modo pwm
}
