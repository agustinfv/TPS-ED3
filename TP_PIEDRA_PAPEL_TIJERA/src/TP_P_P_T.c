#include "LPC17xx.h"
#include <stdio.h>
/*
 * Voy a realizar el juego piedra papel, tijera o piedra.
 *
 * date: 23/03/2026
 * authors: Agustin Vera
 * version 4.0
 */

	//Prototipo de funciones
void configGPIO();

void delay(void);

void resultado(uint8_t jugada_user, uint8_t jugada_LPC);	//Calcula el resultado y da las respectivas salidas
uint8_t matriz_R[3][3] = {		//1 -> ganó el jugador, 0 -> empate, -1 -> ganó el LPC
 { 0, -1, 1},
 { 1, 0, -1},
 {-1, 1, 0}
};
const char* traduccion[] = {"piedra", "papel", "tijera"};	//Variable para remplazar el número con el correspondiente en texto de la jugada

int main(void){
uint8_t baraja[3]= {0, 1, 2};
uint8_t jugada_user = -1;
uint8_t jugada_LPC = -1;

	configGPIO();

	while(1){
		for(uint8_t i= 0 ; i<3 ; i++){			//Baraja las posibles respuestas y se queda con la que justo asignó en el momento
			jugada_LPC = baraja[i];				//en el que usuario hizo su jugada
			if((LPC_GPIO0->FIOPIN & 0x7) != 0){
				break;
			}
		}

		switch(LPC_GPIO0->FIOPIN & 0x7){	//Ya que solo se presiona 1 botón a la vez, podemos usar switch
		case 1:								//Se presionó el 0.0 -> piedra
			jugada_user = 0;
			resultado(jugada_user, jugada_LPC);
			break;
		case 2:								//Se person el 0.1 -> papel
			jugada_user = 1;
			resultado(jugada_user, jugada_LPC);
			break;
		case 4:								//Se presionó el 0.2 -> tijera
			jugada_user = 2;
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
	LPC_PINCON->PINMODE0 |= (0x3F << 0);// pull-down activadas
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

	printf("El usuario ha usado %s\n", traduccion[jugada_user]);
	printf("El microcontrolador ha usado %s\n", traduccion[jugada_LPC]);

	switch(matriz_R[jugada_user][jugada_LPC]){	//De acuerdo a la matriz decide el resultado, enciende el respectivo LED e imprime el texto
	case 0:
		printf("Ha habido un empate\n");
		LPC_GPIO0->FIOSET |= (1<<4);
		delay();
		break;
	case 1:
		printf("Ha ganado el usuario\n");
		LPC_GPIO0->FIOSET |= (1<<5);
		delay();
		break;
	case -1:
		printf("Ha ganado el microcontrolador\n");
		LPC_GPIO0->FIOSET |= (1<<6);
		delay();
		break;
	default:
		printf("Ha ganado Expedition 33\n");	//No debería entrar nunca acá, igual.
		break;

	}
	LPC_GPIO0->FIOCLR |= (7<<4);	//Limpio salidas a espera de otra jugada
	printf("Listo para la siguiente ronda\n");
	return;
}


void delay(void){
	for(uint32_t i=0 ; i<4000000 ; i++){};
}
