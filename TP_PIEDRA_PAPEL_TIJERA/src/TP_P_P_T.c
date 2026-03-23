#include "LPC17xx.h"
#include <stdio.h>
#include <stdint.h>

/*
 * Voy a realizar el juego piedra papel, tijera o piedra.
 *
 * date: 22/03/2026
 * author: Agustin Vera
 * version 4.0 Terminado
 */
#define FIOPINP0 LPC_GPIO0->FIOPIN
#define FIOSETP0 LPC_GPIO0->FIOSET
#define FIOCLRP0 LPC_GPIO0->FIOCLR
int mapa[8] = { -1, 0, 1, -1, 2, -1, -1, -1 };
int resultado[3][3] =
{{ 0, -1, 1},
{ 1, 0, -1},
{-1, 1, 0}};
int index = 0;

void configGPIO();
//int jugadaControlador(int);
void delay(int);
//int matriz(int,int);
void determinarOperacion(int);
void determinarResultado(int);
void imprimirLed(int);

int main(void){
	configGPIO();

	while(1){
		// tengo 8 valores posibles, pero solo los que me sirven son 001(1) piedra, 010(2) papel, 100 (4) tijera

		uint32_t lectura = (~(FIOPINP0)) & 0b111;
		int valorJugador = mapa[lectura];//el valor leido (0 a 7), lo convierto a un valor contenido en la matriz
		if(valorJugador != -1)
		{
			//el usuario presiono un boton, debo esperar que el usuario deje de presionar el boton asique le coloco un delay
			delay(400000);
			printf("Jugada del jugador: \n");
			determinarOperacion(valorJugador);
			int valorControlador = index%3;// valores posibles 0,1,2
			delay(20000);
			printf("Jugada del microcontrolador:\n");
			determinarOperacion(valorControlador);
			int resultadoJugada = resultado[valorJugador][valorControlador];//1 indica que gana el jugador, 0 indica empate y -1 indica que gana el microcontrolador
			delay(20000);
			printf("Resultado de la jugada: \n");
			determinarResultado(resultadoJugada);
			delay(20000);
			lectura = 0;
		}
		index++;
}
}

/*
int jugadaControlador(int index){
	return (index%3);
}
*/
void determinarOperacion(int i){
	switch(i){
	case 0:
		//piedra
		printf("Piedra");
		break;
	case 1:
		printf("Papel");
		break;
	default:
		printf("Tijera");
	}
}

void determinarResultado(int i){
	switch(i){
	case -1:
		printf("Gana el microcntrolador.\n");
		//salida p0.6
		imprimirLed(6);
		break;
	case 0:
		printf("Empate.\n");
		//salida p0.4
		imprimirLed(4);
		break;
	default:
		printf("Gana el jugador. \n");
		//salida p0.5
		imprimirLed(5);
	}
}


void imprimirLed(int i){

	FIOSETP0 = (1 << i);
	delay(500000);
	FIOCLRP0 = (1 << i);
	return;
}

void delay(int tiempo){
	for(uint32_t i = 0; i < tiempo; i++)
	{
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
	//P0.4 EMPATE, P0.5 gana el jugador y P0.6 gana el microcontrolador

	//desergenizo
	LPC_GPIO0->FIOCLR = (0b111<<4);//bits 1110000 (no limpie el p0.3)
}


