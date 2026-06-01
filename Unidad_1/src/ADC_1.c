#include "LPC17xx.h"

/*
 * Configurar modulo ADC de manera que realice conversiones cada 1 seg, la base de tiempo la crearemos
 * con el modulo SysTick. La lectura del dato analogico lo haremos dentro de la rutina de interrup
 * del modulo ADC
 *
 * Conectaremos 3 leds a la placa de desarrollo, y realizaremos la sig logica
 *
 * 1)Si convertimos un dato entre 0V-1V encederemos el primer led (azul)
 * 2)Si convertimos un dato entre 1V-2V encederemos el primer led (verde)
 * 3)Si convertimos un dato entre 2V-3V encederemos el primer led (rojo)
 * 4)Si convertimos un dato mayor a 3 V parpadearán los 3 leds
 */

//(100mSeg * 100MHz)-1 = load ---> 9999999d -> 98967F
//puedo usar un contador dentro de la IRQ del systick para que cuando llegue a 10 ese contador,
// active interrupcion por ADC
volatile uint32_t contador = 1;


void configSysTick();
void configGPIO();
void configADC();

int main(void){
	configGPIO();
	configSysTick();
	configADC();
	NVIC_SetPriority(SysTick_IRQn,3);

	while(1){

	}
}

void configSysTick(){
	SysTick->LOAD = 0x98967F;
	SysTick->VAL = 0;
	SysTick->CTRL= 0x7;
}

void SysTick_IRQHandler(){
	if(contador == 10)
	{
		//activar interrupcion por ADC
		LPC_ADC->ADCR &= ~(0b11<<25);
		LPC_ADC->ADCR |= (1<<24);//inicio conversion
		contador = 1;
	}
	else{
		contador++;
	}
	SysTick->CTRL &= (SysTick->CTRL);//solo coloco porque el profesor pide
}

void configGPIO(){
	//GPIO
	LPC_PINCON->PINSEL0 &= ~(0x3F << 8);//P0.4, P0.5 Y P0.6 config como salidas GPIO

	//analogica quiero usar el canal 0 por lo tanto creo que debo usar AD0.0
	LPC_PINCON->PINSEL1 = (LPC_PINCON->PINSEL1 & ~(1<<15)) | (1<<14);
	LPC_PINCON->PINMODE1 = (LPC_PINCON->PINMODE1 & ~(1<< 14)) | (1<<15);
	//salida digital
	LPC_GPIO0->FIODIR |= (0x7 << 4);

	//desenergizo salida
	LPC_GPIO0->FIOCLR = (0x7 << 4);
}
void configADC(){

	//enciendo periferico
	LPC_SC->PCONP |= (1<<12);//pcadc ACTIVO
	LPC_SC->PCLKSEL0 |= (0b11 << 24);// velocidad ddel periferico lo seteo a 12.5Mhz ( CCLK = 100Mhz)

	//ADC
	LPC_ADC->ADCR |= (1<<0);//quiero realizar conversion en el canal 0 osea AD0.0
	LPC_ADC->ADCR &= ~(1<<16);//modo burst desactivado
	LPC_ADC->ADCR &= ~(0b111 << 24);//start = 000
	LPC_ADC->ADCR |= (1<<21);//PDN activado

	//interrupcion solo quiero que interrumpe un canal en especfico, en este caso seria el AD0.0
	LPC_ADC->ADINTEN &= ~(1<<8);//desactive interrupcion global

	LPC_ADC->ADINTEN |= (1<<0);//habilitada interrupcion por canal 0
	//NVIC
	NVIC_SetPriority(ADC_IRQn,2);
	NVIC_EnableIRQ(ADC_IRQn);
}
void ADC_IRQHandler(){
	//pregunto si se produjo interrupcion por canal 0
	if(LPC_ADC->ADSTAT & (1<<0))
	{
		//asumo que uso 3,3v verf+ y 0v vref-
		//si 4095 son 3.3v, entonces un valor mayor a 3722 produce la 4 secuencia
		uint32_t resultado = (LPC_ADC->ADDR0 & (0xFFF<<4)) >> 4;//lo shiftee y lo vuelvo a shiftear para que
		//el resultado este alineado
		//al leer el resultado, estoy limpiando la bandera de DONE
		if(resultado >= 0xE8A)
		{
			//muestro secuencia 4
		}
		else if(resultado >= 0x9B1  && resultado < 0xE8A)
		{
			//muestro secuencia 3
		}
		else if(resultado >=0x4D8 && resultado < 0x9B1)
		{
			//muestro secuencia 2
		}
		else{
			//muestro secuencia 1
		}
	}

}








