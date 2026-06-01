#include "LPC17xx.h"

/*
 * Configurar el pin P0.4 como entrada digital con resistencia de pull down y
utilizarlo para decidir si el valor representado por los pines P0.0 al P0.3 van a
ser sumados o restados al valor guardado en la variable "acumulador".
El valor inicial de "acumulador" es 0.
 */

int main(void){
	LPC_PINCON->PINSEL0 &= ~(0xFF << 0);
	//LPC_PINCON->PINSEL0 &= ~(0b11<<8);
	LPC_PINCON->PINMODE0 |= (0b11<<8);//habilito pulldown
	LPC_GPIO0->FIODIR &= ~(1<<4);
	//del enunciado interpetro  que P0.0 a P0.3 son entradas
	LPC_GPIO0->FIODIR &= ~(0xF << 0);

	// si P0.4 esta en alto, sumo. Si esta en bajo, resto
	int acumulador = 0;

	while(1)
	{
		int valor = LPC_GPIO0->FIOPIN & 0xF;
		if(LPC_GPIO0->FIOPIN & (1<<4))
		{
			acumulador += valor;
		}
		else{
			acumulador -= valor;
		}
	}


}
