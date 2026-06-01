#include "LPC17xx.h"

/*
 * Resolucion item 2
 * version 1.0
 */
void configGPIO();
void configEINT();
void configSysTick(uint32_t);
uint32_t buscarPromedio(uint8_t*);
uint8_t values[8];//donde cada elemento puede almacenar valor de 0 a 255, no le doy valores suponiendo que un usuario carga los valores

int main(void){
	configGPIO();
	configEINT();
	configSysTick(0b111);
	NVIC_SetPriority(SysTick_IRQn,1);
	while(1)
	{

	}
}

void configGPIO(void){

	LPC_PINCON->PINSEL0 &= ~(0xFFFF<<0);
	LPC_GPIO0->FIODIR |= (0xFF << 0 );
	LPC_GPIO0->FIOCLR = (0xFF << 0);
}

void configSysTick(uint32_t x){
	/*
	 * PeriodoInt = (load+1) * 1/FrecClock
	 * debe desbordar cada 10mS utilizando cclk 62Mhz
	 * (10mS * 62MHz)-1 = load
	 * load = 619999
	 *
	 */
	SysTick->LOAD = 619999;
	SysTick->VAL = 0;
	SysTick->CTRL = (x<<0);
}

void configEINT(){
	//me explicita cual EINT1 usar, P2.11
	LPC_PINCON->PINSEL4 = (LPC_PINCON->PINSEL4 & ~(0b11<<22) | (1<<22)); // pin configurado como EINT1
	LPC_PINCON->PINMODE4 |= (0b11<<22);//Me pide int por flanco ascendente por lo tanto, configuro como pull-dowm
	LPC_SC->EXTMODE |= (1<<1);//activo sensibilidad por cambio de flanco
	LPC_SC->EXTPOLAR |= (1<<1);//activado por flanco ascendente
	LPC_SC->EXTINT = (1<<1);//limpio flag
	NVIC_SetPriority(EINT1_IRQn,2);
	NVIC_EnableIRQ(EINT1_IRQn);
}

void EINT1_IRQHandler(void){
	if(LPC_SC->EXTINT & (1<<1)){
		//SE PRODUJO INT
			if(SysTick->CTRL & (1<<1)){
				//DEBO DESABILITARLO
				configSysTick(0b000);
			}
			else{
				configSysTick(0b111);
			}
	}
	else{
		//ruido

	}
	LPC_SC->EXTINT = (1<<1);
}

void SysTick_Handler(void){
	uint32_t promedio = buscarPromedio(values) & (0xFF<<0);
	LPC_GPIO0->FIOCLR = 0xFF;
	LPC_GPIO0->FIOSET = (promedio<<0);

}

uint32_t buscarPromedio(uint8_t *x){
	uint32_t prom = 0;
	for(volatile int i = 0; i < 8;i++)
	{
		prom += x[i];
	}
	prom = prom/8;
	return prom;
}





