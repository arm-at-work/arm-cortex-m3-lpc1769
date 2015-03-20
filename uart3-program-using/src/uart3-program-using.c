/*
 ===============================================================================
 Name        : uart3-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-03-18
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>
#include <stdlib.h>

struct {
	uint8_t servoSel :1;
	uint8_t servoPosCounter :2;
	uint8_t servoPos[4];
	uint16_t servoPulse :10;
	uint8_t dcMotorSel :1;
	uint8_t dcMotorSpeed[2];
	uint8_t dcMotorDir :7;
	uint8_t dcMotorPulse :4;
	uint8_t dcMotorMod :8;
} motorCtr;

struct {
	uint8_t motorSel :1;
	uint8_t ledSel :1;
	uint8_t step :8;
	uint8_t intro :1;
	uint8_t received :8;
} moduleCtr;

void uart3Config(void);
void uart3Send(char data);
void uart3PutString(const unsigned char *StrPtr);
void initStructs(void);
void gpio_config(void);
void timer0_config(void);
void moduleControl(void);
void motorsControl(void);
void servoControl(void);
void dcMotorControl(void);
void dcMotorSpeedControl(void);
void dcMotorDirectionControl(void);

void UART3_IRQHandler() {
	switch (moduleCtr.step) {
	case 1:
		switch ((uint8_t) LPC_UART3->RBR) {
		case 'm':
			uart3PutString(
					(unsigned char*) "Qual motor deseja acessar?\n s - Servo motor\n d - Motor DC\n # - voltar\n\n");
			moduleCtr.step = 2;
			break;
		case 'l':
			uart3PutString((unsigned char*) "Módulo de LED's\n\n");
			moduleCtr.step = 7;
			break;
		}
		break;
	case 2:
		switch ((uint8_t) LPC_UART3->RBR) {
		case 's':
			uart3PutString(
					(unsigned char*) "Modulo de servo selecionado!\nEscreva a posição para qual deseja enviar o servo (valor de 1 a 180) ou pressione #, para voltar: \n\n");
			moduleCtr.step = 3;
			break;
		case 'd':
			uart3PutString(
					(unsigned char*) "Modulo de motor DC selecionado!\nDeseja controlar a velocidade ou a direção?\n v - velocidade\n d - direção\n # - voltar\n\n");
			moduleCtr.step = 4;
			break;
		case '#':
			moduleCtr.step = 1;
			uart3PutString(
					(unsigned char*) "Qual módulo você deseja acessar?\n m - Motores\n l - Leds\n\n");
			break;
		}
		break;
	case 3:
		moduleCtr.received = ((uint8_t) LPC_UART3->RBR);
		switch (moduleCtr.received) {
		case '#':
			uart3PutString(
					(unsigned char*) "\n\nQual motor deseja acessar?\n s - Servo motor\n d - Motor DC\n # - voltar\n\n");
			moduleCtr.step = 2;
			break;
		default:
			if (motorCtr.servoPosCounter == 3) {
				motorCtr.servoPulse = atoi(motorCtr.servoPos);
				if (motorCtr.servoPulse > 180) {
					motorCtr.servoPulse = 180;
					LPC_TIM0->MR1 = 1124999;
					uart3PutString(
							(unsigned char*) " - valor está acima do permitido servo na posição 180º\n");
				} else {
					LPC_TIM0->MR1 = (999999 + (564 * motorCtr.servoPulse));
					uart3PutString((unsigned char*) " - servo na posição ");
					uart3PutString(motorCtr.servoPos);
					uart3PutString((unsigned char*) "º!\n");
				}
				motorCtr.servoPosCounter = 0;
			} else {
				motorCtr.servoPos[motorCtr.servoPosCounter] =
						moduleCtr.received;
				uart3Send(motorCtr.servoPos[motorCtr.servoPosCounter]);
				++motorCtr.servoPosCounter;
			}
			break;
		}
		break;
	case 4:
		switch ((uint8_t) LPC_UART3->RBR) {
		case 'v':
			uart3PutString((unsigned char*) "Digite valor de 0 a 7:\n");
			moduleCtr.step = 5;
			break;
		case 'd':
			uart3PutString(
					(unsigned char*) "Digite 'f' para frente ou 't' para traz, ou 'p' para pará-lo:\n");
			moduleCtr.step = 6;
			break;
		case '#':
			uart3PutString(
					(unsigned char*) "\n\nQual motor deseja acessar?\n s - Servo motor\n d - Motor DC\n # - voltar\n\n");
			moduleCtr.step = 2;
			break;
		}
		break;
	case 5:
		moduleCtr.received = ((uint8_t) LPC_UART3->RBR);
		switch (moduleCtr.received) {
		case '#':
			uart3PutString(
					(unsigned char*) "\n\nModulo de motor DC selecionado!\nDeseja controlar a velocidade ou a direção?\n v - velocidade\n d - direção\n # - voltar\n\n");
			moduleCtr.step = 4;
			break;
		default:
			motorCtr.dcMotorSpeed[0] = moduleCtr.received;
			motorCtr.dcMotorPulse = atoi(motorCtr.dcMotorSpeed);
			if (motorCtr.dcMotorPulse > 7) {
				motorCtr.dcMotorPulse = 7;
				uart3PutString(motorCtr.dcMotorSpeed);
				uart3PutString(
						(unsigned char*) " - valor está acima do permitido, motor em velocidade 7\n");
				LPC_TIM0->MR2 = (285714 * motorCtr.dcMotorPulse);
			} else {
				uart3PutString((unsigned char*) "Motor com velocidade ");
				uart3PutString(motorCtr.dcMotorSpeed);
				uart3PutString((unsigned char*) "!\n");
				LPC_TIM0->MR2 = (1999999 - (285714 * motorCtr.dcMotorPulse));
			}
			break;
		}
		break;
	case 6:
		switch ((uint8_t) LPC_UART3->RBR) {
		case 'f':
			uart3PutString((unsigned char*) "Motor movendo-se para frente!\n");
			LPC_GPIO0->FIOPIN = ((LPC_GPIO0->FIOPIN & ~(1 << 0)) | (1 << 6)); // estado lógico 0 nos pinos
			break;
		case 't':
			uart3PutString((unsigned char*) "Motor movendo-se para traz!\n");
			LPC_GPIO0->FIOPIN = ((LPC_GPIO0->FIOPIN & ~(1 << 6)) | (1 << 0)); // estado lógico 0 nos pinos
			break;
		case 'p':
			uart3PutString((unsigned char*) "Motor parado!\n");
			LPC_GPIO0->FIOPIN &= ~(1 << 0 | 1 << 6); // estado lógico 0 nos pinos
			break;
		case '#':
			uart3PutString(
					(unsigned char*) "Modulo de motor DC selecionado!\nDeseja controlar a velocidade ou a direção?\n v - velocidade\n d - direção\n # - voltar\n\n");
			moduleCtr.step = 4;
			break;
		}
		break;
	case 7:
		switch ((uint8_t) LPC_UART3->RBR) {
		case '#':
			moduleCtr.step = 1;
			uart3PutString(
					(unsigned char*) "Qual módulo você deseja acessar?\n m - Motores\n l - Leds\n\n");
			break;
		default:

			break;
		}
		break;
	}
	LPC_UART3->RBR; // limpa registrador de recepção
}

void TIMER0_IRQHandler(void) {
	if (LPC_TIM0->IR & (1 << 0)) { // verifica se flag de interrupção por comparação do canal 0 do timer0 está setada
		LPC_GPIO0->FIOPIN |= (1 << 1); // estado lógico 1, no pino do servo
		LPC_TIM0->IR |= (1 << 0); // limpa flag de interrupção por comparação no canal 0 do timer0
	} else if (LPC_TIM0->IR & (1 << 1)) { // verifica se flag de interrupção por comparação do canal 1 do timer0 está setada
		LPC_GPIO0->FIOPIN &= ~(1 << 1); // estado lógico 1, no pino
		LPC_TIM0->IR |= (1 << 1); // limpa flag de interrupção por comparação no canal 1 do timer0
	} else if (LPC_TIM0->IR & (1 << 2)) { // verifica se flag de interrupção por comparação do canal 2 do timer0 está setada
		LPC_GPIO2->FIOPIN &= ~(1 << 4); // estado lógico 1, no pino do motor DC
		LPC_TIM0->IR |= (1 << 2); // limpa flag de interrupção por comparação no canal 2 do timer0
	} else if (LPC_TIM0->IR & (1 << 3)) { // verifica se flag de interrupção por comparação do canal 3 do timer0 está setada
		LPC_GPIO2->FIOPIN |= (1 << 4); // estado lógico 1, no pino do motor DC
		LPC_TIM0->IR |= (1 << 3); // limpa flag de interrupção por comparação no canal 3 do timer0
	}
}

int main(void) {
	SystemInit();
	initStructs();
	uart3Config();
	gpio_config(); // configura pinos como MAT
	timer0_config(); // configura timer0
	uint32_t wait;
	for (wait = 0; wait < 1000000; wait++)
		;
	uart3PutString(
			(unsigned char*) "Qual módulo você deseja acessar?\n m - Motores\n l - Leds\n\n");
	do { // loop infinito
	} while (1);
	return 0;
}

/** configura pinos como MAT */
void gpio_config(void) {
	/** pino servo */
	LPC_PINCON->PINSEL0 &= ~(0b11 << 2); // configura pino como GPIO
	LPC_GPIO0->FIODIR |= (1 << 1); // pino como saída
	/** pinos motor DC */
	LPC_PINCON->PINSEL0 &= ~(0b11 << 0 | 0b11 << 12); // configura pinos como GPIO
	LPC_GPIO0->FIODIR |= (1 << 0 | 1 << 6); // pino como saída
	LPC_GPIO0->FIOPIN &= ~(1 << 0 | 1 << 6); // estado lógico 0 nos pinos
	/** pinos pulso motor DC */
	LPC_PINCON->PINSEL4 &= ~(0b11 << 8); // pino como GPIO
	LPC_GPIO2->FIODIR |= (1 << 4); // pino como saída
	LPC_GPIO2->FIOPIN |= (1 << 4); // pino com estado lógico alto
}

/** função configura UART3 */
void uart3Config(void) {
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
	NVIC->IP[8] |= (12 << 3); // nível de prioridade alto para interrupções na UART3
	NVIC->ISER[0] |= (1 << 8); // habilita interrupção para a UART3
}

void uart3Send(char data) {
	do { // bit Trasmitter Holding Register Empty - enquanto o bit não estiver limpo
	} while (!(LPC_UART3->LSR & (1 << 5))); // bit Trasmitter Holding Register Empty - enquanto o bit não estiver limpo
	LPC_UART3->THR = data; //
}

void uart3PutString(const unsigned char *StrPtr) {
	while (*StrPtr != '\0') {
		uart3Send(*StrPtr);
		StrPtr++;
	}
}

void initStructs(void) {
	motorCtr.servoPosCounter = 0;
	moduleCtr.step = 1;
}

/** configura timer0 */
void timer0_config(void) {
	LPC_SC->PCONP |= (1 << 1); // liga timer0
	LPC_SC->PCLKSEL0 = ((LPC_SC->PCLKSEL0 & ~(0b11 << 2)) | (0b01 << 2)); // atribui clock de C_CLK (100MHz)
	LPC_TIM0->CTCR &= ~(0b11 << 0); // incremento ocorre na borda da subida de PCLK_TIMER0
	LPC_TIM0->PR = 1; // valor do prescaler
	LPC_TIM0->MR0 = 999999; // valor para comparação no canal 0
	LPC_TIM0->MR1 = 1074249; // valor para comparação no canal 0 - 1023499/1074249/1124999 - 564
	LPC_TIM0->MR2 = 1; // valor para reinicialização do contador do timer
	LPC_TIM0->MR3 = 1999999; // valor de pulso inicial para motor DC (duty cycle = 0%);
	LPC_TIM0->MCR |= (0b01 << 10); // reinicia-se contador, após coincidência de comparação no canal 3
	LPC_TIM0->EMR &= ~(0b11 << 4 | 0b11 << 6 | 0b11 << 8 | 0b10 << 10); // inverte-se estado lógico no pino MAT, após coincidência de comparação no canal 0
	LPC_TIM0->MCR |= (1 << 0 | 1 << 3 | 1 << 6 | 1 << 9); // habilita interrupção por comparação nos canais 0, 1 e 2 do timer0
	NVIC->IP[1] |= (0 << 3); // nível de prioridade 0 e subnível 0, para timer1 (NVIC->IP[1])
	NVIC->ISER[0] |= (1 << 1); // habilita interrupções no timer0
	NVIC->ICPR[0] |= (1 << 1); // retira, se existente, qualquer interrupção pendente do timer0
	LPC_TIM0->TCR = 1; // inicializa timer0
}

