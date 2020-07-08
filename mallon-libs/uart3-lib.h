/*
 ===============================================================================
 Name        : uart3-lib.h
 Author      : Thiag Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-01-13
 Description : main definition
 ===============================================================================
 */

#ifndef UART3_LIB_H_
#define UART3_LIB_H_

#include "lpc17xx.h"

void uart3Init(void);
void uart3Send(unsigned char);
void uart3PutValue(unsigned char*);

/** função configura e inicializar uart3 */
void uart3Init(void) {

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
}

/** função envia valor de memória de posição de ponteiro */
void uart3PutValue(unsigned char* dataPtr) {
	while (*dataPtr != '\0') {
		uart3Send(*dataPtr);
		dataPtr++;
	}
}

/** função envia dados */
void uart3Send(unsigned char data) {
	do { // espera até que que registrador de transmissão possa receber novos dados
	} while (!(LPC_UART3->LSR & (1 << 5)));
	LPC_UART3->THR = data; // coloca bit no registrador de envio
}

//Redireciona a saída formatada.
int __sys_write(int iFileHandler, char *pcBuffer, int iLength) {
	unsigned int i;
	for (i = 0; i < iLength; i++) {
		uart3Send(pcBuffer[i]);
	}
	return iLength;
}

#endif /* UART3_LIB_H_ */
