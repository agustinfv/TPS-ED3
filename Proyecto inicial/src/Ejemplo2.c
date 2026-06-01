#include "LPC17xx.h"

/*Consigna:
Enciende y apaga un led conectado al pin p0.22 de forma intermitente.
El tiempo de retardo es controlado por una señal digital proveniente del PIN2.10
configurado como entrada digital.
*/

void delay();
int main(void){

	LPC_PINCON->PINSEL1 &= ~(0b11<<12);
	LPC_PINCON->PINSEL4 &= ~(0b11<<20);

	LPC_GPIO0->FIODIR |= (0b01<<22); //Puerto 0, pin 22 configurado como salida
	LPC_GPIO2->FIODIR &= ~(0b01<<10); //Puerto 2, pin 10 del pinsel 2 configurado como entrada

	//no necesito activar la mascara del P2.10 porque supongo que la señal proviene del pin fisico

	LPC_GPIO0->FIOCLR = (0b01<<22);//P0.22, apague led
	//recordar que en la entrada, solo puedo recibir 1 ALTO o 0 BAJO

	while(1)
	{
		int tiempo;

		if(LPC_GPIO2->FIOPIN & (1 << 10))
		{
			tiempo = 1000000; // rápido
		}
		else
		{
			tiempo = 3000000; // lento
		}

		LPC_GPIO0->FIOSET = (1 << 22);
		delay(tiempo);
		LPC_GPIO0->FIOCLR = (1 << 22);
		delay(tiempo);
	}
}

void delay(int tiempo)
{
	for(volatile int i = 0; i < tiempo; i++);
}
