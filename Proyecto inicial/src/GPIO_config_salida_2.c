/*
 * En los pines P2.0 a P2.7 se encuentra conectado un display de 7 segmentos.
 * Utilizando la variable numDisplay [10] ={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67}
 * que codifica los números del 0 a 9 para ser mostrados en el display,
 * realizar un programa que muestre indefinidamente la cuenta de 0 a 9 en dicho display.
 */

#include "LPC17xx.h"

void delay();
int main(void)
{
	int numDisplay [10] ={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};
	LPC_PINCON->PINSEL4 &= ~(0xFFFF<<0);//declaro como GPIO, recordar que son 32 bits, no 16
	LPC_GPIO2->FIODIRL = (0xFF);//establezco como salida
	LPC_GPIO2->FIOCLR = (0xFF);// coloco en bajo todos los pines del 0 al 7

	while(1){
		delay();
		for(volatile int i=0; i <= 9; i++)
		{
		LPC_GPIO2->FIOCLRL = (0xFF);// limpio los bits anteriores
		LPC_GPIO2->FIOSETL = (numDisplay[i]);// por el puerto saco los valores almacenados en la matriz
		delay();
		}//una vez que llega al valor maximo de la matriz, reinicio
	}
}
void delay(){
	for(volatile int i=0; i < 1000000; i++)
	{

	}
}
