#include "LPC17xx.h"
/*
 * Enciende y apaga un led conectado al pin P0.22, de manera indefinida,
 * con un periodo de intermitencia cada 100mS
 * Implementar retardo via desbordamiento del Systick
 * author: Agustin Vera
 * date: 30/03/2026
 * version 1.0
 */
	//Formula: PeriodoInt = (SystickLoad + 1) * PeriodoClock
	//100mS / (1/100MHz) -1 = SystickLoad
	uint32_t val = 0x98967F;
void configGPIO();
void configSystick();

int main(void){
	configGPIO();
	configSystick();
	while(1)
	{

	}
}

void configGPIO()
{
	//GPIO
	LPC_PINCON->PINSEL1 &= ~(0b11<<12);
	LPC_PINCON->PINMODE1 &= ~(0b11<<12);
	//salida
	LPC_GPIO0->FIODIR |= (1<<22);
	//desenergizo
	LPC_GPIO0->FIOCLR = (1<<22);

}
void configSystick(){

	SysTick->LOAD = val;
	SysTick->VAL = 0;
	SysTick->CTRL= (0x7<<0);// recordar que es un registro de 32 bits
}
void SysTick_Handler(void){
	if(~(LPC_GPIO0->FIOPIN) & (1<<22))
	{
		LPC_GPIO0->FIOSET = (1<<22);
	}
	else
	{
		LPC_GPIO0->FIOCLR = (1<<22);
	}
	SysTick->CTRL &= SysTick->CTRL; // LIMPIO COUNTFLAG
}
