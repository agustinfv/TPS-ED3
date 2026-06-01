#include "LPC17xx.h"
/*
 * Enciende y apaga un led conectado al pin p0.22, con un periodo de intermitencia determinado
 * por el estado de un switch ubicado en el pin p2.10
 *
 * details: Implementar interrupciones externas
 * author: Agustin Vera
 * date: 20/03/2026
 * version 1.0.0
 */
volatile uint8_t cambio = 0;



void configGPIO();
void configIntEXT();
void delay(uint32_t);

int main(void){
	configGPIO();
	configIntEXT();
	while(1)
	{
	if(cambio%2)
	{
		delay(100000);
		LPC_GPIO0->FIOSET = (1<<22);
		delay(100000);
		LPC_GPIO0->FIOCLR = (1<<22);
	}
	else{
		delay(400000);
		LPC_GPIO0->FIOSET = (1<<22);
		delay(400000);
		LPC_GPIO0->FIOCLR = (1<<22);
	}

}
}

void configGPIO(){
	//GPIO
	LPC_PINCON->PINSEL1 &= ~(0b11<<12);//P0.22

	//EXT
	LPC_PINCON->PINSEL4 &= ~(1<<21);//P2.10
	LPC_PINCON->PINSEL4 |= (1<<20);//P2.10 configure EINT0
	LPC_PINCON->PINMODE4 &= ~(0b11<<20);//p2.10 pull up activada

	//entrada
	LPC_GPIO2->FIODIR &= ~(1<<10);//p2.10

	//salida
	LPC_GPIO0->FIODIR |= (1<<22);//p0.22

	//apago el led
	LPC_GPIO0->FIOCLR = (1<<22);

}
void configIntEXT(){
	LPC_SC->EXTMODE |= (1<<0);//flanco
	LPC_SC->EXTPOLAR &= ~(1<<0);//descendente
	LPC_SC->EXTINT = (1<<0);// limpio bandera
	NVIC_SetPriority(EINT0_IRQn,3);
	NVIC_EnableIRQ(EINT0_IRQn);
}

void EINT0_IRQHandler(void){

	if(LPC_SC->EXTINT & (1<<0)){
		//ocurrio interrupcion
		cambio++;
		if(cambio == 0xFE)
		{
			cambio= 0;
		}
		LPC_SC->EXTINT = (1<<0);// limpio bandera

	}
}

void delay(uint32_t tiempo){
	for(volatile uint32_t t = 0; t < tiempo ; t++)
	{

	}
}












