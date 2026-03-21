#include "LPC17xx.h"
/*
 * Voy a realizar el juego piedra papel, tijera o piedra. Implementando interrupciones (no estoy seguro de esta
 * desicion ya que los requisitos no lo mencionan)
 *
 * date: 21/03/2026
 * author: Agustin Vera
 * version 2.0
 */

void configGPIO();

void delay();

int main(void){
	configGPIO();

	while(1){

	}
}
void configGPIO(){
	//GPIO voy a utilizar el puerto 0.
	LPC_PINCON->PINSEL0 &= ~(0x3F3F << 0);//recordar que no se usa el P0.3
	LPC_PINCON->PINMODE0 &= ~(0x3F3F << 0 );// pull-up activadas
	//mascaras?

	//entrada
	LPC_GPIO0->FIODIR &= ~(0b111<<0);//Pines desde el p0.0 al p0.2

	//salida
	LPC_GPIO0->FIODIR |= (0b111<<4);//Pines comprendidos desde el p0.4 al p0.6

	//desergenizo
	LPC_GPIO0->FIOCLR = (0b111<<4);//bits 1110000 (no limpie el p0.3)
}


