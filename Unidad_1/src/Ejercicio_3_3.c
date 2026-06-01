#include "LPC17xx.h"
/*
 * consigna:
 * Realizar un programa que configure el puerto P2.0 y P2.1 para que provoquen una interrupcion por flanco de subida para el primer pin
 * y por flanco de bajada para el segundo. Cuando la interrupcion sea por P2.0 se enviara por el pin P0.0 la secuencia de bits 010011010.
 * Si la interrupcion es por P2.1 se enviara por el pin P0.1 la secuencia 011100110. Las secuencias se envian unicamente cuando se produce
 * una interrupcion, en caso contrario la salida de los pines tienen valores 1 logicos. Que prioridad tienen configuradas por defecto
 * estas interrupcion?
 *
 *Nota author: las salidas por los pines p0.0 y p0.1 tienen activada su pull up
 *
 * date: 24/03/2026
 * Author Agustin Vera
 * version 1.0.0
 *
 */
int secuenciaP0_0[9] = {0,1,0,0,1,1,0,1,0};
int secuenciaP0_1[9] = {0,1,1,1,0,0,1,1,0};
void configGPIO();
void configIntGPIO();// el enunciado no dice que son interrupciones externas por lo tanto asumo que son por GPIO
void mostrarSecuencia(int*,int);
int main(void){
	configGPIO();
	configIntGPIO();

	while(1)
	{

	}

}
void configGPIO(){
	//GPIO
	LPC_PINCON->PINSEL0 &= ~(0xF << 0); //P0.0 y P0.1
	LPC_PINCON->PINSEL4 &= ~(0xF << 0); // P2.0 y P2.1

	//MODE, P2.0 flanco de subida(puldown) y P2.1 flanco de bajada (pullup)
	LPC_PINCON->PINMODE0 &= ~(0xF << 0); //pull activada en P0.0 y P0.1
	LPC_PINCON->PINMODE4 |= (0b11 << 0);//pull down en P2.0
	LPC_PINCON->PINMODE4 &= ~(0b11 << 2); // pull up en P2.1

	//salidas
	LPC_GPIO0->FIODIR |= (0b11 << 0); //P0.0 y P0.1

	//entradas
	LPC_GPIO2->FIODIR &= ~(0b11 << 0); //P2.0 y P2.1

	//energizo
	LPC_GPIO0->FIOSET = (0b11 << 0);//P0.0 y P0.1
}

void configIntGPIO(){
	//P2.0
	LPC_GPIOINT->IO2IntClr = (1<<0); //limpio bandera
	LPC_GPIOINT->IO2IntEnR = (1<<0); //habilito

	//P2.1 descendente
	LPC_GPIOINT->IO2IntClr = (1<<1);//limpio bandera
	LPC_GPIOINT->IO2IntEnF = (1<<1);//descendente quiere decir que es pull up

	//permisos
	NVIC_EnableIRQ(EINT3_IRQn);

}

void EINT3_IRQHandler(void){
	if(LPC_GPIOINT->IO2IntStatR & (1<<0))
	{
		//interrupcion por P2.0
		mostrarSecuencia(secuenciaP0_0,0);
		LPC_GPIOINT->IO2IntClr = (1<<0);
		LPC_GPIO0->FIOSET = (0b11 << 0);//ENERGIZO SALIDA
	}else if(LPC_GPIOINT->IO2IntStatF & (1<<1))
	{
		//int por P2.1
		mostrarSecuencia(secuenciaP0_1,1);
		LPC_GPIOINT->IO2IntClr = (1<<1);//limpio bandera
		LPC_GPIO0->FIOSET = (0b11 << 0);//ENERGIZO SALIDA
	}else{
		//ruido
	}
}
void mostrarSecuencia(int* array,int pin)
{
	for(volatile int i = 0; i < 9 ; i++)
	{
		if(array[i] & (1<<0))
		{
		LPC_GPIO0->FIOSET = (1<<pin);
		}
		else
		{
		LPC_GPIO0->FIOCLR = (1<<pin);
		}
	}
}





