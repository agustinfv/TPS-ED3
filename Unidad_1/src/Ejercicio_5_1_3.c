#include "LPC17xx.h"

/*
 * Consigna:
 * Escribir el codigo que configure el timer0 para cumplir con las especificaciones dadas
 * en la figura adjunta. Considerar una frecuencia de cclk de 100MHz y una division de reloj de periferico de 2
 *
 *author Agustin Vera
 *date 19/04/2026
 *version 1.0
 */

void configTMR0();

int main(void){

	configTMR0();
	while(1){

	}
}

void configTMR0(){
	LPC_SC->PCONP |= (1<<1);
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & ~(1<<2)) | (1<<3);
	LPC_TIM0->CTCR &= ~(0b11<<0);
	LPC_TIM0->PR = 2;
	LPC_TIM0->TCR |= (1<<1);//habilito counter reset
	LPC_TIM0->TCR &= ~(1<<1);//desabilito counter reset

	LPC_TIM0->IR = (1<<0);//limpio lag
	LPC_TIM0->MCR |= (1<<0);
	LPC_TIM0->MCR |= (1<<1);
	LPC_TIM0->MR0 = 6;
	LPC_TIM0->TCR |= (1<<0);//habilito interrupcion
	NVIC_EnableIRQ(TIMER0_IRQn);


}
