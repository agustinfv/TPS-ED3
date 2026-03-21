#include "LPC17xx.h"
/*
 * Voy a realizar el juego piedra papel, tijera o piedra. Implementando interrupciones (no estoy seguro de esta
 * desicion ya que los requisitos no lo mencionan)
 *
 * date: 21/03/2026
 * author: Agustin Vera
 * version 2.0
 */
#define FIOPINP0 LPC_GPIO0->FIOPIN
int mapa[8] = { -1, 0, 1, -1, 2, -1, -1, -1 };
int resultado[3][3] =
{{ 0, -1, 1},
{ 1, 0, -1},
{-1, 1, 0}};
int index = 0;

void configGPIO();
int jugadaControlador(int);
void delay();
int matriz(int,int);


int main(void){
	configGPIO();

	while(1){
		// tengo 8 valores posibles, pero solo los que me sirven son 001(1) piedra, 010(2) papel, 100 (4) tijera
		switch(~(FIOPIN & 0b111)){
		int valorControlador = jugadaControlador(index);// valores posibles 0,1,2
		int ResultadoJugada;
		}
		case 1:
			//piedra
			ResultadoJugada = matriz(0,valorControlador);
			break;
		case 2:
			//papel
			ResultadoJugada = matriz(1,valorControlador);
			break;
		case 4:
			//tjera
			ResultadoJugada = matriz(2,valorControlador);
			break;
		default:
			//no hace nada
		}
		index++;
}


int jugadaControlador(int index){
	return (index%3);
}

int matriz( int valorUsuario,int valorControlador)
{
	//1 indica que gana el jugador, 0 indica empate y -1 indica que gana el microcontrolador
	return resultado[valorUsuario][valorControlador];

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


