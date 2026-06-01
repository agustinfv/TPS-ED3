#include "LPC17xx.h"
/*
 * Configurar la interrupcion externa EINT1 para que interrumpa por flanco de bajada y la interrupcion
 * EINT2 para que interrumpa por flanco de subida. En la interrupcion por flanco de bajada configurar
 * el systick para que desborde cada 25mS, mientras que en la interrupcion por flanco de subida
 * configurarlo para que desborde cada 60mS, Considerar que EINT1 tiene mayor prioridad que EINT2
 * date 31/03/2026
 * author Agustin Vera
 * version 1.0
 */

/*Formula matematica
 * PeriodoInt = (Load + 1) * PeriodoClock
 * (25mS * 100MHz) -1 = Load25
 * (60ms * 100mhz) -1 = Load60
 */
volatile uint32_t load25 = 0x26259F;//2499999
volatile uint32_t load60 = 0x5BD8D7F;// 5999999

void configGPIO();
void configSysTick(uint32_t);
void configInt();

int main(void){
	configGPIO();
	configInt();
	//configSysTick();

	while(1){

	}
}

void configGPIO(void){
	//EINT1
	LPC_PINCON->PINSEL4 &= ~(1<<23);
	LPC_PINCON->PINSEL4 |= (1<<22);//activado (01) P2.11

	//EINT2
	LPC_PINCON->PINSEL4 &= ~(1<<25);
	LPC_PINCON->PINSEL4 |= (1<<24);//activado (01) P2.12

	//modos, coloco a EINT1 como pull up y a EINT2 como pull down
	LPC_PINCON->PINMODE4 &= ~(0b11<<22);// EINT1
	LPC_PINCON->PINMODE4 |= (0b11<<24);// EINT1

	//entrada
	LPC_GPIO2->FIODIR &= ~(0b11<<11);// P2.11 y P2.12 colocados como entradas
}

void configInt(void){

	LPC_SC->EXTMODE |= (1<<1); //dejo cambio por flanco
	LPC_SC->EXTMODE |= (1<<2); //al parecer solo puedo cambiar 1 bit a la vez, aplica lo mismo para POLAR
	LPC_SC->EXTPOLAR &= ~(1<<1);//dejo activo por flanco de bajada EINT1
	LPC_SC->EXTPOLAR |= (1<<2);// activo por flanco de subida EINT2
	//limpio banderas
	LPC_SC->EXTINT = (0b11 << 1);
	NVIC_SetPriority(EINT1_IRQn,1);//tiene mas prioridad que EINT2
	NVIC_SetPriority(EINT2_IRQn,2);
	NVIC_EnableIRQ(EINT1_IRQn);
	NVIC_EnableIRQ(EINT2_IRQn);//habilito interrupciones

}

void configSysTick(volatile uint32_t x){
	SysTick->LOAD = x;
	SysTick->VAL = 0;
	SysTick->CTRL = (0x07<<0);


}

void EINT1_IRQHandler(){
	if(LPC_SC->EXTINT & (1<<1))
	{
		configSysTick(load25);
	}
	else
	{

	}
	LPC_SC->EXTINT |= (1<<1);//limpio flag
}
void EINT2_IRQHandler(){
	if(LPC_SC->EXTINT & (1<<2))
	{
		configSysTick(load60);
	}
	else
	{

	}
	LPC_SC->EXTINT |= (1<<2);//limpio flag
}

void SysTick_Handler(){
	//como el ejercicio no explicita nada, lo unico que voy a hacer es desactivar la int por SysTick
	SysTick->CTRL = (0b000<<0);
	//SysTick->CTRL &= SysTick->CTRL;// solo coloco esta linea porque el profesor me pide de esta forma
}
