#include "LPC17xx.h"

/*
 * consigna: Realizar un programa que configure el puerto P0.0 y P2.0 para que provoquen una interrupcion por flanco de subida
 * Si la interrupcion es por P0.0 guardar el valor binario 100111 en la variable 'auxiliar', si es por P2.0 guardar el valor
 * binario 111001011010110
 * author: Agustin Vera
 * date 27/03/2026
 * version 1.0.0
 */
volatile uint32_t aux = 0;

void configGPIO();
void configInt();

int main(void){
	configGPIO();
	configInt();

	while(1){

	}

}

void configGPIO(){
	//GPIO
	LPC_PINCON->PINSEL0 &= ~(0b11<<0);//P0.0
	LPC_PINCON->PINSEL4 &= ~(0b11<<0);//P2.0
	//mode
	LPC_PINCON->PINMODE0 |= (0b11<<0);//pull down en P0.0
	LPC_PINCON->PINMODE4 |= (0b11<<0);//pull down en P2.0
	//entradas
	LPC_GPIO0->FIODIR0 &= ~(1<<0);//P0.0
	LPC_GPIO2->FIODIR0 &= ~(1<<0);//P2.0
}

void configInt(){
	//limpieza de bandera
	LPC_GPIOINT->IO0IntClr = (1<<0);// P0.0
	LPC_GPIOINT->IO2IntClr = (1<<0);// P2.0
	//habilitacion
	LPC_GPIOINT->IO0IntEnR |= (1<<0);//P0.0
	LPC_GPIOINT->IO2IntEnR |= (1<<0);//P2.0
	//NVIC
	NVIC_SetPriority(EINT3_IRQn,3);
	NVIC_EnableIRQ(EINT3_IRQn);
}
void EINT3_IRQHandler(void){
	if(LPC_GPIOINT->IO0IntStatR & (1<<0))
	{
		//interrrupcion por P0.0
		aux = (0b100111);
		LPC_GPIOINT->IO0IntClr = (1<<0);// P0.0
	}
	else if(LPC_GPIOINT->IO2IntStatR & (1<<0)){
		//interrupcion por P2.0
		aux = (0b111001011010110);
		LPC_GPIOINT->IO2IntClr = (1<<0);// P2.0
	}else{
		//ruido
	}
}
