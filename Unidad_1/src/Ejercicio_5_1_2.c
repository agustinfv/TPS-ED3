#include "LPC17xx.h"

/*
 * Calcular cual es el tiempo maximo que se puede temporizar utilizando un timer en modo match
 * como maximo valor de prescaler y maximo divisor de frecuencia del periferico. Especificar el valor
 * a cargar en los correspondientes registros del timer. Suponer una frecuencia de core cclk 50 MHz
 */

//Voy a practicar de configurar el timer, utilizare match0.1

int main(void){

}
void configTMR(){
	//enciendo
	LPC_SC->PCONP |= (1<<1);
	//cclk 50 MHz y me pide el maximo divisor de frecuencia, asumo PCLKSEL0 bit 01
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (1<<3)) | (1<<2);
	LPC_PINCON->PINSEL7 = (LPC_PINCON->PINSEL7 | (1<<21)) & ~(1<<20);//MAT0.1

	LPC_TIM0->CTCR &= ~(0b11 << 0);

	//valor maximo de prescalador 2^32 - 1
	LPC_TIM0->PR = 4294967295;

	//habilitacion y desactivacion del counter reset
	LPC_TIM0->TCR |= (1<<1);
	LPC_TIM0->TCR &= ~(1<<1);

	//bandera
	LPC_TIM0->IR = (1<<0);

	//interrumpo y paro en MRO
	LPC_TIM0->MCR = (LPC_TIM0->MCR | (1<<0)) | (1<<2);

	LPC_TIM0->MR0 = 4294967295;

	LPC_TIM0->TCR |= (1<<0);//habilito interrupcion
	NVIC_EnableIRQ(TIMER0_IRQn);
	/*
	 * Calculos:
	 * 1/cclk = 1tic --> 20nS
	 *
	 * tMatch = (1 / (cclk/n)) * ( 4294967295 + 1) * (4294967295 +1)
	 * 1/(50MHz) * ...
	 * Deberia interrumpir cada 0.38e12 segundos
	 *
	 * 1 min 60seg
	 * X --- 0.3689e12 ->>> X = 6148333333 minutos. Segun mis calculoss, debe interrumpir cada X segundos
	 */


}
