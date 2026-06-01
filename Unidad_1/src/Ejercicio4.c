#include "LPC17xx.h"
#define PIN0 LPC_GPIO0->FIOPIN
/*
 * Considerando pulsadores normalmente abiertos conectados en un extremo a
masa y en el otro directamente a las entradas P0.0 y p0.1. Realizar un
programa que identifique en una variable cual o cuales pulsadores han sido
presionados. Las identificaciones posibles a implementar en esta variable van a
ser "ninguno", "pulsador 1", "pulsador 2", "pulsador 1 y 2"
 */

int main(void){
	//GPIO
	LPC_PINCON->PINSEL0 &= ~(0xF << 0);
	LPC_PINCON->PINMODE0 &= ~(0xF << 0); // habilito pull up
	//ENTRADA, recordar que puedo leer 1 o 0
	LPC_GPIO0->FIODIR0 &= ~(0b11<<0);//P: 0 y 1 como entrada

	// pulsador 1 es igual al pin p0.0
	// pulsador 2 es igual al pin p0.1
	// recordar que 0 es presionado, y 1 es no presionado
	while(1){
		volatile int variable = 0;// 0 ninguno, 1 pulsador1, 2 pulsador2, 3 ambos presionados

		if(!(PIN0 & 1<<0) && !(PIN0 & (1 << 1))){
			//ambos estan presionados
			variable = 3;
		}else if(!(PIN0 & 1<<0)){
			//esta presionado el pulsador 1
			variable =1;
		}else if(!(PIN0 & (1<<1))){
			//esta presionado pulsador 2
			variable =2;
		}else{
			//ninguno esta presionado
			variable =0;
		}
	}
}
