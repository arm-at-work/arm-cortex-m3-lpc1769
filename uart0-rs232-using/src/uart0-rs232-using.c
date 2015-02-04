/*
 ===============================================================================
 Name        : uart0-rs232-using.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-01-27
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

//__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

#define Liga 1
#define Desliga 0

#define LED_A(L_D) LPC_GPIO0->FIOPIN = (LPC_GPIO0->FIOPIN&~(1<<22))|(L_D<<22)
#define LED_B(L_D) LPC_GPIO0->FIOPIN = (LPC_GPIO0->FIOPIN&~(1<<21))|(L_D<<21)

//Protótipos de função
void gpioConfig();
void UART0Config();
void UART0EscreveCaractere(char c);
void UART0EscreveMensagem(const char *string);

void UART0_IRQHandler() {
	switch ((unsigned char) LPC_UART0->RBR) {

	case 'a':
		LED_A(Liga);
		UART0EscreveMensagem("Liga led azul\n");
		break;
	case 's':
		LED_A(Desliga);
		UART0EscreveMensagem("Desliga led azul\n");
		break;
	case 'd':
		LED_B(Liga);
		UART0EscreveMensagem("Liga led verde\n");
		break;
	case 'f':
		LED_B(Desliga);
		UART0EscreveMensagem("Desliga led verde\n");
		break;
	}
}

int main(void) {
	SystemInit();
	NVIC_SetPriorityGrouping(0b010);
	gpioConfig();
	UART0Config();
	do { // loop infinito
	} while (1);
	return 0;
}

void gpioConfig() {
	LPC_PINCON->PINSEL1 &= ~(0b11 << 12 | 0b11 << 10); // configura pinos como GPIO
	LPC_GPIO0->FIODIR |= 1 << 22 | 1 << 21; // configura pinos como saída
}

void UART0Config() {
	LPC_SC->PCONP |= 1 << 3; //Ativa a UART 0

	LPC_PINCON->PINSEL0 |= (1 << 4 | 1 << 6); // configura pinos como TXD0 e RXD0 respectivamente

	LPC_SC->PCLKSEL0 &= ~(0b11 << 6); // PLCK_UART 0 = C_CLK/4 = 25MHz
	//Baud Rate = 9600bps
	LPC_UART0->LCR = 1 << 7; // DLAB = 1 - permite acesso aos registradores U3DLL E U3DLM (registradores que armazenam o valor do divisor para o baudrate)
	LPC_UART0->DLL = 163; // U3DLL = 163 - valor obtido do cálculo 25MHz / (16 * 9600) = 162,76 = 163
	LPC_UART0->DLM = 0; // U3DLM = 0
	LPC_UART0->FDR = 0 | 1 << 4; /** DIVADDVAL = 0 E MULVAL = 1 - verificando-se o valor
	 fracionado: (25MHz / (16 * 9600 * 162)) -1 = 0,00469 o valor de divisor/multiplicador que mais
	 se aproxima é o 0, sendo, então os valores do DIVADDVAL E MULVAL (divisores fracionários) serão 0 e 1
	 respectivamente (verificar na tabela do próprio fabricante) */
	LPC_UART0->LCR &= ~(1 << 7); // DLAB = 0 - bloqueia acesso aos registradores DLL e DLM

	/* UART0 - 8 bits de dados (0b11<<0)
	 1 bit de parada (0<<2)
	 Sem paridade (0<<3) */
	LPC_UART0->LCR = 0b11 << 0 | 0 << 2 | 0 << 3;

	LPC_UART0->THR = 0; // limpa o registro de transmissão
	LPC_UART0->RBR; // limpa o registro de recepção

	// Nível de disparo 1 - DMA */
	LPC_UART0->FCR &= ~(0b11 << 6);

	LPC_UART0->IER |= (1 << 0); // habilita interrupção RBR

	LPC_UART0->FCR |= 1 << 0 | 1 << 1 | 1 << 2; // ativa e reinicia as FIFOs da UART0
	NVIC_SetPriority(UART0_IRQn, 1); // seta nível de prioridade alto para a UART0
	NVIC_EnableIRQ(UART0_IRQn); // habilita interrupções na UART0
}

/** função escreve caractere na UART */
void UART0EscreveCaractere(char c) {
	do { // aguarda até que o registrador U0THR esteja vazio
	} while (!(LPC_UART0->LSR & (1 << 5)));
	LPC_UART0->THR = c; // atribui dado ao registrador, o que o colocará no topo da FIFO de transmissão
}

/** função escreve string na UART */
void UART0EscreveMensagem(const char *string) {
	while (*string != '\0') { // verifica se string chegou a fim
		UART0EscreveCaractere(*string); // envia valoar da posição de memória do ponteiro, para a função que envia caracteres para a FIFO de transmissão
		string++; // incrementa posição de memória do ponteiro
	}
}

