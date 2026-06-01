#include "LPC17xx.h"

/*
 * Resolucion item 1
 * version 1.0
 */

void configINTGPIO();
volatile static uint32_t valor;

int main(void){
configINTGPIO();

while(1){

}


}

void configINTGPIO(){
	LPC_PINCON->PINSEL4 &= ~(0xFFFF<<0);
	//LPC_PINCON->PINMODE4 |= (0xFFFF<<0);//pulldown activada, CREO QUE ES MEJOR LA CONFIGURACION SIN pull up-down (PINMODE valor 10)
	LPC_PINCON->PINMODE4 = ((LPC_PINCON->PINMODE4 & ~(0xFFFF)) | 0b1010101010101010<<0);
	LPC_GPIO2->FIODIR &= ~(0xFF<<0);

	LPC_GPIOINT->IO2IntClr = (0xFF<<0);//limpio la bandera
	LPC_GPIOINT->IO2IntEnR |= (0xFF<<0);//habilito interrupcion
	LPC_GPIOINT->IO2IntEnF |= (0xFF<<0);//habilito interrupcion
	NVIC_EnableIRQ(EINT3_IRQn);

}

void EINT3_IRQHandler(void){
	if(LPC_GPIOINT->IOIntStatus & (1<<2)){
		//ocurrio una interrupcion
		valor = (LPC_GPIO2->FIOPIN & ~(0xFF << 16));//primero limpio los bits 31-16,
		//y luego y el valor almacenado en los pines 0-15 los guardo en la variable
		valor = (LPC_GPIO2->FIOPIN);
	}
	else{
		//ruido
	}
	LPC_GPIOINT->IO2IntClr = (0xFF<<0);//LIMPIO LAS BANDERAS
}
