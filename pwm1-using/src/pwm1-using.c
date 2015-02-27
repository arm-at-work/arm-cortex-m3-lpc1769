/*
 ===============================================================================
 Name        : pwm1-using.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-02-22
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código à memória flash

/* protótipos */
void pwm1Config(void); // configura pwm1

int main(void) {
	SystemInit(); // inicializa sistema - c_clk = 100mhz, pclk = 25mhz, etc.
	SystemCoreClockUpdate(); // atualiza valor de clock da cpu (c_clk) na variável SystemCoreClock
	pwm1Config(); // configura pwm1
	do { // loop infinito
	} while (1);
	return 1;
}

/* configura pwm1 */
void pwm1Config(void) {
	/* configurações gerais */
	LPC_SC->PCONP |= (1 << 6); // habilita periférico pwm1
	LPC_SC->PCLKSEL0 = ((LPC_SC->PCLKSEL0 & ~(0b11 << 12)) | (0b01 << 12)); // pclk_pwm1 = c_clk
	LPC_PINCON->PINSEL3 = ((LPC_PINCON->PINSEL3 & ~(0b11 << 4)) | (0b10 << 4)); // pino como pwm1[1]
	/* configurações de operação */
	LPC_PWM1->CTCR &= ~(0b11 << 0); // fonte de incremento do contador TC é pclk_pwm1
	LPC_PWM1->PR = 1; // atribui valor ao prescaler
	LPC_PWM1->MR0 = 999999; // atribui valor do período do pwm, no caso, para que seja gerada borda de subida à cada 20ms, ou 50Hz
	LPC_PWM1->MR1 = 500000; // atribui valor de comparação ao canal 1, para borda de descida em 0,47ms
	LPC_PWM1->MCR = ((LPC_PWM1->MCR & ~(0b11 << 1 | 0b11 << 4)) | (0b01 << 1)); // reinicia contador, após coincidência de comparação no canal 0
	LPC_PWM1->MCR |= (1 << 0); // habilita evento de interrupção após coincidência de comparação no canal 0
	LPC_PWM1->PCR |= (1 << 9); // habilita saída no pino pwm1[1]
	LPC_PWM1->LER |= (1 << 1); // habilita posterior atualização no valor de comparação do canal 1
	LPC_PWM1->TCR |= (1 << 0 | 1 << 3); // habilita contadores; modo pwm
}

