/*
 ===============================================================================
 Name        : main.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-03-20
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

/** leds */
#define LED1 (1 << 22)
#define LED2 (1 << 21)
#define LED3 (1 << 3)

/** botões */
#define BUTTON1 (1 << 6)
#define BUTTON2 (1 << 7)
#define BUTTON3 (1 << 8)

// sem proteção contra leitura de código
//__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

void ledConfig(void); // função configura pinos utilizados nos leds
void buttonConfig(void); // função configura pinos utilizados nos push buttons

/** função reservada - manipula interrupções geradas */
void EINT3_IRQHandler() {
	// botão 1?
	if (LPC_GPIOINT->IO2IntStatF & (BUTTON1)) { // verifica se flag de estouro associada ao pino, está setada
		LPC_GPIO0->FIOPIN ^= (LED1); // inverte nível lógico do pino
	}
	// botão 2?
	if (LPC_GPIOINT->IO2IntStatF & (BUTTON2)) {
		LPC_GPIO0->FIOPIN ^= (LED2); // inverte nível lógico do pino
	}
	// botão 3?
	if (LPC_GPIOINT->IO2IntStatF & (BUTTON3)) {
		LPC_GPIO0->FIOPIN ^= (LED3); // inverte nível lógico do pino
	}
	// Limpa flags de estouro, dos pinos usados para disparo
	LPC_GPIOINT->IO2IntClr = (BUTTON1 | BUTTON2 | BUTTON3);
}

int main(void) {
	SystemInit(); // configura recursos básicos do microcontrolador - C_CLK = 100MHz
	NVIC_SetPriorityGrouping(0b010); // determina valor do PRIGROUP = 0b010
	ledConfig(); // configura pinos utilizados nos leds
	buttonConfig(); // configura pinos utilizados pelos push buttons
	do { // loop infinito
	} while (1);
	return 0;
}

/** realiza configurações dos leds*/
void ledConfig(void) {
	/* configura LED1 e LED2*/
	LPC_PINCON->PINSEL1 &= ~(0b11 << 12 | 0b11 << 10); // configura pino P0.22 como GPIO
	LPC_GPIO0->FIODIR |= (1 << 22 | 1 << 21); // configura pino GPIO como saída
	/* configura LED3 */
	LPC_PINCON->PINSEL0 &= ~(0b11 << 6); // configura pino P0.22 como GPIO
	LPC_GPIO0->FIODIR |= (1 << 3); // configura pino GPIO como saída
}

/** realiza configurações relativas aos botões */
void buttonConfig(void) {
	LPC_PINCON->PINSEL4 &= ~(0b11 << 16 | 0b11 << 14 | 0b11 << 12); // configura pinos P2.8, P2.7 e P2.6 como GPIO
	LPC_GPIO2->FIODIR &= ~(BUTTON1 | BUTTON2 | BUTTON3); // configura pinos P2.10, P2.8 e P2.7 como entrada
	LPC_PINCON->PINMODE4 &= ~(0b11 << 16 | 0b11 << 14 | 0b11 << 12); // habilita pullup nos pinos P2.8, P2.7 e P2.6
	/* habilita interrupção para borda da descida */
	LPC_GPIOINT->IO2IntEnF |= (BUTTON1 | BUTTON2 | BUTTON3); // IO2IntEnF (Falling edge) - I02IntEnR (Raising Edge)
	/* como as interrupções do GPIO são associadas à interrupção EINT3 (External Interrupt)
	 precisamos habilita a mesma, para que a captura funcione. Colocaremos essa interrupção com nível de prioridade
	 alto. */
	NVIC_SetPriority(EINT3_IRQn, 1); // nível de prioridade alto
	NVIC_EnableIRQ(EINT3_IRQn); // habilita interrupção
	LPC_GPIOINT->IO2IntClr |= (BUTTON1 | BUTTON2 | BUTTON3); // limpa flags de interrupção dos pinos dos push buttons
}
