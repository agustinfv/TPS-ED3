/*
 * Consiga:
 * Enciende y apaga un led conectado al pin P0.22, cuando el tiempo entre 2 eventos consecutivos
 * asociados a un flanco ascendente en el pin P1.26 debido a la interrupcion del TMR0,
 * sea mayor o igul a 1seg
 *
 * author: Agustin Vera
 * date 13/04/2026
 * version 1.0
 */

#include "LPC17xx.h"

void configPIN();
void configTMR0();
volatile static uint32_t firstEvento = 0;
volatile static uint32_t CRAnterior;


int main(void){
	configPIN();
	configTMR0();
	while(1){

	}
}
void configPIN(){
	LPC_PINCON->PINSEL1 &= ~(0b11<<12);
	LPC_PINCON->PINSEL3 |= (0b11<<20);//P1.26 modo CAP0.0
	LPC_PINCON->PINMODE3 |= (0b11<<20);//pull-down activado en P1.26

	LPC_GPIO0->FIODIR |= (1<<22);//P0.22 como salida
	LPC_GPIO0->FIOCLR = (1<<22);//desenergizo salida

}

void configTMR0(){
	LPC_SC->PCONP |= (1<<1);//enciendo periferico
	LPC_SC->PCLKSEL0 &= ~(0b11<<2);// cclk = 25Mhz
	LPC_TIM0->CTCR &= ~(0b11<<0);//timer mode

	//valor PR
	LPC_TIM0->PR = 0;//lo dejo en cero para tener una maxima presicion, cada tick dura 1/cclk

	//habilitacion y deshabilitacion del counter reset
	LPC_TIM0->TCR |= (1<<1);
	LPC_TIM0->TCR &= ~(1<<1);

	//limpio flag
	LPC_TIM0->IR = (1<<4);

	//registro de captura
	LPC_TIM0->CCR = (LPC_TIM0->CCR | (1<<0)) | (1<<2);//flanco ascendente y habilito interrupcion

	//habilito tmr0
	LPC_TIM0->TCR |= (1<<0);
	NVIC_EnableIRQ(TIMER0_IRQn);

}

void TIMER0_IRQHandler(){
		if(LPC_TIM0->IR & (1<<4))
		{
			//ocurrio una captura, TC = CR
			//conozco que 1/cclk -> 1/25MHz = 40 nS

			if(firstEvento == 0)
			{
				firstEvento = 1;
				CRAnterior = LPC_TIM0->CR0;
			}
			else{
				uint32_t CRActual = LPC_TIM0->CR0;
				CRAnterior = CRActual;
				uint32_t T2_T1 = CRActual - CRAnterior;
				//debo convertir este valor a seg
				//si 40nS es un 1tic, entonces 1/40nS son 25e6 tics
				if(T2_T1 > 25e6)
				{
					//prendo o apago el led dependiendo del estado en el que se encuentre
				}
				else{
					//si entro aqui significa que paso menos de 1 seg entre evento y evento
				}

			}

		}
		LPC_TIM0->IR = (1<<4);
}












