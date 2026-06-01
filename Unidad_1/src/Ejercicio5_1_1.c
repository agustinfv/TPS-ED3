/*
 * Consigna:
 * Enceder y apagar un led conectado al pin P0.22, de manera indefinida, con un periodo de
 * intermitencia de 1s. Implementar retardo via desbordamiento del TMR0
 * author: Agustin Vera
 * date: 12/04/2026
 * version 1.0
 *
 */

#include "LPC17xx.h"

void configGPIO();
void configTMR();//intermitencia cada 1s

int main(void){
	configGPIO();
	configTMR();

	while(1){

	}
}

void configGPIO(){

	LPC_PINCON->PINSEL1 &= ~(0b11 << 12);
	LPC_PINCON->PINSEL3 |= (0b11<<24);//P1.28 MAT0.0
	//salida
	LPC_GPIO0->FIODIR |= (1<<22);

	//limpio
	LPC_GPIO0->FIOCLR = (1<<22);

}

void configTMR(){
	LPC_SC->PCONP |= (1<<1);//enciendo periferico
	LPC_SC->PCLKSEL0 &= ~(0b11<<2);//selecciono frecuencia cclk/4
	LPC_TIM0->CTCR &= ~(0b11<<0);//PC matchea con PR, y ocasiona que se incremente TC
	LPC_TIM0->PR = 6249999;
	LPC_TIM0->TCR &= ~(1<<1);//COUNTER RESET = 0

	//COMO ESTOY TRABAJANDO CON TIMER0, LIMPIO LA BANDERA MR0
	LPC_TIM0->IR = (1<<0);

	LPC_TIM0->MR0 = 3;//VALOR CON EL QUIERO QUE MATCHEE TC
	LPC_TIM0->MCR |= (1<<0);//HABILITO INTERRUPCION POR MATCHEO de TC y MR
	LPC_TIM0->MCR |= (1<<1);//RESETEO TC SI MATCHEA CON MR
	LPC_TIM0->EMR &= ~(0b11<<0);//NINGUNA MODALIDAD EN P1.28
	LPC_TIM0->TCR |= (1<<0);//COUNTER ENABLE = 1, interrupcion por TMR0
	NVIC_EnableIRQ(TIMER0_IRQn);

}
void TIMER0_IRQHandler(){
	if(LPC_TIM0->IR & (1<<0))
	{
		if(LPC_GPIO0->FIOPIN & (1<<22))
		{
			LPC_GPIO0->FIOCLR = (1<<22);//APAGO LED
		}
		else{
			LPC_GPIO0->FIOSET = (1<<22);//PRENDO LED
		}
	}
	LPC_TIM0->IR = (1<<0);//limpio bandera
}
