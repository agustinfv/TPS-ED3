#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
//enciende y apaga un led conectado al pin P0.22 de forma interminente

void delay();

int main(void){

	LPC_PINCON->PINSEL1 &= ~(1<<6);//utilizo modulo GPIO

	LPC_PINCON->PINMODE1 |= (1<<22); //coloque modo repetidor en el pin 22

	LPC_GPIO0->FIODIR |= (1<<22);



	while(1)
	{
		delay();
		LPC_GPIO0->FIOCLR |= (1<<22);
		delay();
		LPC_GPIO0->FIOSET |= (1<<22);
	}

	return 0;

}
void delay()
{
	for(int i = 0; i < 1000;i++)
	{

	}
}
