#include "LPC17xx.h"

/*
 * Consigna
 * Escribir un programa para que por cada presion de un pulsador, la frecuencia de parpadeo
 * de un led disimuya a la mitad debido a la modificacion del prescaler del timer2. El pulsador debe
 * producir una interrupcion por EINT1 con flanco descendente
 */

void configEINT1();
void configTMR2();
void seteoPR(uint32_t);

int main(void){
	configEINT1();
	configTMR2();
	while(1){

	}
}

void configEINT1(){
	LPC_PINCON->PINSEL4 = (LPC_PINCON->PINSEL4 & ~(0b11<<22)) | (1<<22);//EINT1
	LPC_PINCON->PINMODE4 &= ~(0b11<<22);//pull up

	LPC_SC->EXTMODE |= (1<<1);//activo nivel por flanco
	LPC_SC->EXTPOLAR &= ~(1<<1);//activo por bajo
	NVIC_SetPriority(EINT1_IRQn,1);
	NVIC_EnableIRQ(EINT1_IRQn);
}

void EINT1_IRQHandler(void){
	//se pulso el boton, por lo tanto debo disminuir a la mitad el PR
	if(LPC_SC->EXTINT & (1<<1)){
		if(LPC_TIM2->PR < 4000000)
		{
			seteoPR(LPC_TIM2->PR);
		}
		else{
			//vuelvo a condicion inicial, es decir que interrumpa cada 5 seg
			seteoPR((25000/2) -1);
		}
	}
	else{
		//ruido
	}
	LPC_SC->EXTINT = (1<<1);//limpio flag
}

void configTMR2(){
	LPC_SC->PCONP |= (1<<22);
	LPC_SC->PCLKSEL1 &= ~(0b11<<12);//como el ejercicio no explicita la frecuencia, asumo 100MHZ/4
	//Quiero practicar usando la salida MAT2.0
	LPC_PINCON->PINSEL0 |= (0b11<<12);//salida MAT2.0
	LPC_GPIO0->FIOCLR = (1<<6);//limpio la salida

	LPC_TIM2->CTCR &= ~(0b11<<0);
	//como el ejercicio no me explicita un valor de PR, quiero comenzar produciendo un toggle cada 5 seg
	//PR = PCLK * (t [uS]/1e6) --> 25MHz * (1000/1e6) - 1 == 24999
	seteoPR((25000 / 2)-1);
	LPC_TIM2->TCR |= (1<<1);
	LPC_TIM2->TCR &= ~(1<<1);

	LPC_TIM2->IR = (1<<0);
	LPC_TIM2->MCR |= (0b11<<0);

	//tiempo de interrupcion tmatch = (tdelay)/tover --> 5000 - 1
	LPC_TIM2->MR0 = 4999;//MI PPREGUNTA ES INTERRUMPE CADA 5 SEG?
	LPC_TIM2->EMR |= (0b11<<4);//mi intencion es producir toggle automatico
	LPC_TIM2->TCR |= (1<<0);//habilito interrupcion
	NVIC_SetPriority(TIMER2_IRQn,2);
	NVIC_EnableIRQ(TIMER2_IRQn);

}
void seteoPR(uint32_t x)
{
	LPC_TIM2->PR = x*2;

}

void TIMER2_IRQHandler(){
	LPC_TIM2->IR = (1<<0);//LIMPIO FLAG
}
