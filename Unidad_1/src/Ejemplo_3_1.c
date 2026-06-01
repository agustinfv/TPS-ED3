#include "LPC17xx.h"
/*
 * Enciende y apaga un led coenctado al pin P0.22, con un periodo de intermitencia determinado por el estado
 * de un switch ubicado en el pin P0.15 La intermitencia que le voy a colocar sera, alternar entre 2 estados,
 * 1 estado led apagado (no ocurrio interrupcion) y 2 estado led parpadeando (ocurrio interrupcion)
 * Implementar Interrupciones por GPIO
 * date: 20/03/2026
 * author: Agustin Vera
 * version 1.0
 */

volatile uint8_t estado = 0; // led apagado, estado = 1 led debe parpadear
volatile uint8_t index =0; //lo utilizo para cuando index llegue a cierto valor, apago led
void delay();
void confGPIO();
void configIntGPIO();

int main(void){
	confGPIO();
	configIntGPIO();
while(1){
	if(estado & (1 << 0))
	{
		//led debe parpadear, ocurrio interrupcion
		LPC_GPIO0->FIOSET = (1<<22);
		delay();//1 segundo = 1000mS
		LPC_GPIO0->FIOCLR = (1<<22);
		delay();
		index++;
		if(index == 0xFE)
			{
					estado = 0;
					index = 0;
			}
	}else
	{
	}
}
}

void confGPIO(){
	//GPIO
	LPC_PINCON->PINSEL1 &= ~(0b11 << 12);//P0.22
	LPC_PINCON->PINSEL0 &= ~(0b11 << 30);//P0.15
	LPC_PINCON->PINMODE0 &= ~(0b11 << 30);//habilite pull up para p0.15

	//entrada
	LPC_GPIO0->FIODIR &= ~(1 << 15);//P0.15

	//salida
	LPC_GPIO0->FIODIR |= (1 << 22);//P0.22

	//desenergizo
	LPC_GPIO0->FIOCLR = (1 << 22);// apague la salida del led
}
void configIntGPIO(){
	LPC_GPIOINT->IO0IntClr = (1<<15);//limpie la bandera de interrupcion para el pin p0.15
	LPC_GPIOINT->IO0IntEnF |= (1<<15);// habilito interrupcion por flanco descendente en p0.15
	NVIC_SetPriority(EINT3_IRQn,1);//estableci prioridad de 1
	NVIC_EnableIRQ(EINT3_IRQn);// habilite interrupcion por GPIO
}

void EINT3_IRQHandler(void){
	  if((LPC_GPIOINT->IO0IntStatF) & (1<<15))
	  {
		  estado = 1;
		  LPC_GPIOINT->IO0IntClr |= (1<<15);//limpio bandera de interrupcion
	  }
}
void delay(){
	for(volatile uint32_t t = 0; t < 100000; t++ ){

	}
}
