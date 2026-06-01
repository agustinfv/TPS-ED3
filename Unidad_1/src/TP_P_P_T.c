#include "LPC17xx.h"
/*
 * Voy a realizar el juego piedra papel, tijera o piedra. Implementando interrupciones (no estoy seguro de esta
 * desicion ya que los requisitos no lo mencionan)
 *
 * date: 21/03/2026
 * author: Agustin Vera
 * version 1.0
 */
#define StateFlagIn LPC_GPIOINT->IO0IntStatF // significa leo algun bit asociado a la bandera de entrada
#define ClearFlagIn LPC_GPIOINT->IO0IntClr // significa limpio alguna bandera de la entrada

void configGPIO();
void configIntGPIO();
void delay();

int main(void){
	configGPIO();
	configIntGPIO();

	while(1){

	}
}
void configGPIO(){
	//GPIO voy a utilizar el puerto 0.
	LPC_PINCON->PINSEL0 &= ~(0x3F3F << 0);//recordar que no se usa el P0.3
	LPC_PINCON->PINMODE0 &= ~(0x3F3F << 0 );// pull-up activadas
	//mascaras?

	//entrada
	LPC_GPIO0->FIODIR &= ~(0b111<<0);//Pines desde el p0.0 al p0.2

	//salida
	LPC_GPIO0->FIODIR |= (0b111<<8);//Pines comprendidos desde el p0.4 al p0.6

	//limpio el puerto
	LPC_GPIO0->FIOCLR = (0x77<<0);//bits 1110111 (no limpie el p0.3)
}

void configIntGPIO(){
	//recordar que tengo pull up activada, por lo tanto el flanco deberia ser descendente
	LPC_GPIOINT->IO0IntClr = (0b111<<0); //limpie banderas de las entradas
	LPC_GPIOINT->IO0IntEnF &= ~(0b111<<0);//habilite interrup de las entradas
	NVIC_SetPriority(EINT3_IRQn,3);
	NVIC_EnableIRQ(EINT3_IRQn);
}

void EINT3_IRQHandler(void){
	if(StateFlagIn & (1<<0))
	{
		//interrumpio P0.0
		ClearFlagIn = (1<<0);

	}else if(StateFlagIn & (1<<1))
	{
		//interrumpio P0.1
		ClearFlagIn = (1<<1);
	}else if(StateFlagIn & (1<<2))
	{
		//interrumpio p0.2
		ClearFlagIn = (1<<2);
	}else{
		//sucedio ruido, no hago nada
	}
}
