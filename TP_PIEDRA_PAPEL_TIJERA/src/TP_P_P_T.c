#include "LPC17xx.h"
#include <stdlib.h>
/*
 * Voy a realizar el juego piedra papel, tijera o piedra.
 *
 * date: 21/03/2026
 * author: Agustin Vera
 * version 2.0
 */

	//Prototipo de funciones
void configGPIO();

void delay();

void resultado(uint8_t jugada_user, uint8_t jugada_LPC);

int main(void){
uint8_t baraja[3]= {1, 2, 4};
uint8_t jugada_user = 0;
uint8_t jugada_LPC = 0;

	configGPIO();

	while(1){
		for(uint8_t i= 0 ; i<3 ; i++){			//Baraja las posibles respuestas y se queda con la que justo asignó en el momento
			jugada_LPC = baraja[i];				//en el que usuario hizo su jugada
			if((LPC_GPIO0->FIOPIN & 0x7) != 7){
				break;
			}
		}

		switch((~LPC_GPIO0->FIOPIN) & 0x7){	//Ya que solo se presiona 1 botón a la vez, podemos usar switch. Negado por lógica nevativa
		case 1:								//Se presionó el 0.0 -> piedra
			jugada_user = 1;
			resultado(jugada_user, jugada_LPC);
			break;
		case 2:								//Se person el 0.1 -> papel
			jugada_user = 2;
			resultado(jugada_user, jugada_LPC);
			break;
		case 4:								//Se presionó el 0.2 -> tijera
			jugada_user = 4;
			resultado(jugada_user, jugada_LPC);
			break;
		default:							//En caso default no hago nada, el programa queda en espera.
			break;
		}

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
	return;
}

void resultado (uint8_t jugada_user, uint8_t jugada_LPC){


	return;
}


void delay(void){
	for(uint32_t i=0 ; i<4000000 ; i++){};
}
