/*
 ===============================================================================
 Name        : spi-test.c
 Author      : Thiago Mallon <thiagomallon@gmail.com>
 Version     :
 Created at  : 2015-05-28
 Description : main definition
 ===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>
#include "characters.h"

#define LCD_COMMAND 0 // instrução para envio de comando
#define LCD_DATA 1 // instrução para envio de dado

#define GLCD_X 84
#define GLCD_Y 48

void pinsConfig(void); // configura pinos
void spiInit(void); // configura/inicializa spi
void spiSend(unsigned char data); // envia dado via spi
void glcdPutChar(char character);
void glcdClear(void);
void glcdInit(void);
void glcdPutString(char *characters);
void glcdSend(uint8_t dataOrCommand, uint8_t data);

int main(void) {
	SystemInit(); // inicializa o sistema - c_clk = 100mhz, pclk = 25mhz, etc
	spiInit(); // configura/inicializa spi
	pinsConfig();
	glcdInit();
	glcdClear();
	glcdPutString("GLCD test");
	do {
	} while (1);

	return 1;
}

/** configura/inicializa spi */
void spiInit(void) {
	/** configurações gerais */
	LPC_SC->PCONP |= 1 << 8; // habilita periférico spi
	LPC_SC->PCLKSEL0 &= ~(0b11 << 16); // pclk_spi = c_clk / 4
	/** configurações de operação */
	LPC_SPI->SPCR |= (1 << 5); // modo master;
	LPC_SPI->SPCR &= ~(1 << 6 | 1 << 4 | 1 << 3 | 1 << 2); // transferência pelo msb; CPOL = 1; CPHA = 1; dado de 8 bits
}

/** envia dados */
void spiSend(unsigned char data) {
	LPC_SPI->SPDR = data; // envia um dado ou instrução
	do { // aguarda fim da transferência
	} while (!(LPC_SPI->SPSR & (1 << 7)));
}

/** lê dados */
unsigned char spiRead(void) {
	LPC_SPI->SPDR = 0x00;
	do { // aguarda fim da transfência
	} while (!(LPC_SPI->SPSR & (1 << 7)));
	return LPC_SPI->SPDR; // retorna o dado armazenado no buffer de recepção
}

void glcdPutChar(char character) {
	unsigned int index;
	glcdSend(LCD_DATA, 0x00);
	for (index = 0; index < 5; index++) {
		glcdSend(LCD_DATA, ASCII[character - 0x20][index]);
	}
	glcdSend(LCD_DATA, 0x00);
}

void glcdClear(void) {
	unsigned int index;
	for (index = 0; index < GLCD_X * GLCD_Y / 8; index++) {
		glcdSend(LCD_DATA, 0x00);
	}
}

void glcdInit(void) {
	LPC_GPIO0->FIOPIN &= ~(1 << 24);
	LPC_GPIO0->FIOPIN |= (1 << 24); // alterna pino RST
	glcdSend(LCD_COMMAND, 0x21);  // comandos extendidos.
	glcdSend(LCD_COMMAND, 0xB1);  // seta contrast do glcd (Vop)
	glcdSend(LCD_COMMAND, 0x04);  // seta coeficiente. //0x04
	glcdSend(LCD_COMMAND, 0x14);  // seta modo bias de 1:48. //0x13
	glcdSend(LCD_COMMAND, 0x20);  // comandos básicos
	glcdSend(LCD_COMMAND, 0x0C);  // glcd no modo normal
}

void glcdPutString(char *characters) {
	while (*characters) {
		glcdPutChar(*characters++);
	}
}

void glcdSend(uint8_t dataOrCommand, unsigned char data) {
	if (dataOrCommand == 1) { // verifica se envio sera de dado ou de comando
		LPC_GPIO0->FIOPIN |= (1 << 16); // estado lógico 1 - instrução para envio de dado
	} else {
		LPC_GPIO0->FIOPIN &= ~(1 << 16); // estado lógico 0 - instrução para envio de comando
	}
	LPC_GPIO0->FIOPIN &= ~(1 << 23); // estado lógico 0
	spiSend(data); // envia dado ou comando para o slave
	LPC_GPIO0->FIOPIN |= (1 << 23); // estado lógico 0
}

/** configura pinos */
void pinsConfig(void) {
	// MOSI - master output slave input
	LPC_PINCON->PINSEL1 |= (0b11 << 4); // pino como MOSI
	LPC_PINCON->PINMODE1 =
			((LPC_PINCON->PINMODE1 & ~(0b11 << 4)) | (0b10 << 4)); // desabilita pull-ups e pull-downs dos pinos
	// SCLK - clock
	LPC_PINCON->PINSEL0 |= (0b11 << 30); // pino como SCLK
	LPC_PINCON->PINMODE0 = ((LPC_PINCON->PINMODE0 & ~(0b11 << 30))
			| (0b10 << 30)); // desabilita pull-ups e pull-downs dos pinos
	// DC - quando low espera comando, quando high espera dado
	LPC_PINCON->PINSEL1 &= ~(0b11 << 0); // pino como gpio
	LPC_GPIO0->FIODIR |= (1 << 16); // pino como saída
	LPC_GPIO0->FIOPIN &= ~(1 << 16); // estado lógico 0
	// CE - quando low permite recepção de dados, quando high apenas exibe
	LPC_PINCON->PINSEL1 &= ~(0b11 << 14); // pino como gpio
	LPC_GPIO0->FIODIR |= (1 << 23); // pino como saída
	LPC_GPIO0->FIOPIN |= (1 << 23); // estado lógico 0
	// RST - quando high, reseta o glcd
	LPC_PINCON->PINSEL1 &= ~(0b11 << 16); // pino como gpio
	LPC_GPIO0->FIODIR |= (1 << 24); // pino como saída
	LPC_GPIO0->FIOPIN |= (1 << 24); // estado lógico 0
}
