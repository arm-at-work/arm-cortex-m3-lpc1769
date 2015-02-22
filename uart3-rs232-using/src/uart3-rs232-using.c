/*
 ===============================================================================
 Name        : uart3-rs232-using.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-02-16
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h> // implementa proteção de código da memória flash

//__CRP const unsigned int CRP_WORD = CRP_NO_CRP;

//Protótipos de função
void ledsConfig();
void uart3Init();
void uart3PutChar(char c);
void uart3PutString(const char *string);

void UART3_IRQHandler() {
	switch ((unsigned char) LPC_UART3->RBR) {
	case 'a':
		LPC_GPIO0->FIOPIN |= (1 << 22);
		uart3PutString("Liga led azul\n");
		break;
	case 's':
		LPC_GPIO0->FIOPIN &= ~(1 << 22);
		uart3PutString("Desliga led azul\n");
		break;
	case 'd':
		LPC_GPIO0->FIOPIN |= (1 << 21);
		uart3PutString("Liga led verde\n");
		break;
	case 'f':
		LPC_GPIO0->FIOPIN &= ~(1 << 21);
		uart3PutString("Desliga led verde\n");
		break;
	}
}

int main(void) {
	SystemInit();
	SCB->AIRCR = 0x05FA0000 | 0b100 << 8; // prigroup = b100
	ledsConfig();
	uart3Init();
	do { // loop infinito
	} while (1);
	return 0;
}

void ledsConfig() {
	LPC_PINCON->PINSEL1 &= ~(0b11 << 12 | 0b11 << 10); // configura pinos como GPIO
	LPC_GPIO0->FIODIR |= 1 << 22 | 1 << 21; // configura pinos como saída
}

void uart3Init() {
	/** configurações gerais */
	LPC_SC->PCONP |= (1 << 25); // habilita a UART3
	LPC_SC->PCLKSEL1 &= ~(0b11 << 18); // clock da UART3 será C_CLK / 4 = 25MHz - por padrão, porém, escrito para deixar explícito
	LPC_PINCON->PINSEL9 |= (0b11 << 24 | 0b11 << 26); // configura pinos P4.28 e P4.29 como TX e RX respectivamente, da UART3
	/** configurações taxa baud rate */
	LPC_UART3->LCR |= (1 << 7); // bit DLAB (Divisor Latch Access bit) habilita leitura/escrita dos registrador U3DLL e U3DLM
	LPC_UART3->DLL = 163; // cálculo: 25MHz / (16 * 9600) = 162,760
	LPC_UART3->DLM = 0;
	LPC_UART3->FDR |= (0 | 1 << 4); // cálculo: (25MHz / (16 * 9600 * 163)) -1 = -0,00146
	LPC_UART3->LCR &= ~(1 << 7); // bloqueia leitura/escrita dos registradores U3DLL e U3DLM
	/** configurações do pacote */
	LPC_UART3->LCR |= (0b11 << 0); // caracter de 8 bits
	LPC_UART3->LCR &= ~(1 << 2 | 1 << 3); // 1 bit de parada; Sem bit de paridade
	LPC_UART3->FCR |= (1 << 0 | 1 << 1 | 1 << 2); // ativa e reinicia FIFOS da UART3
	LPC_UART3->THR = 0; // limpa registrador de transmissão
	LPC_UART3->RBR; // limpa registrador de buffer de recepção
	LPC_UART3->FCR &= ~(0b11 << 6); // DMA desabilitado - nível de disparo 1 dado
	/** configurações de interrupção */
	LPC_UART3->IER |= (1 << 0); // habilita interrupção por RBR
	NVIC->IP[8] |= (0 << 5 | 0 << 3); // nível de prioridade alto para interrupções na UART3
	NVIC->ISER[0] |= (1 << 8); // habilita interrupção para a UART3
}

/** função escreve caractere na UART */
void uart3PutChar(char c) {
	do { // aguarda até que o registrador U0THR esteja vazio
	} while (!(LPC_UART3->LSR & (1 << 5)));
	LPC_UART3->THR = c; // atribui dado ao registrador, o que o colocará no topo da FIFO de transmissão
}

/** função escreve string na UART */
void uart3PutString(const char *string) {
	while (*string != '\0') { // verifica se string chegou a fim
		uart3PutChar(*string); // envia valoar da posição de memória do ponteiro, para a função que envia caracteres para a FIFO de transmissão
		string++; // incrementa posição de memória do ponteiro
	}
}

