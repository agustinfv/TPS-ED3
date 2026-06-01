#include "LPC17xx.h"
/*
 * 1)Configure el SysTick Timer de modo que genere una forma de onda llamada PWM.
 * Esta señal debe ser extraida por el pin P0.22 para que controle la intensidad de
 * brillo del led. El periodo de la señal debe ser de 10mS con un duty cycle de 10%
 * Configure la interrupcion externa EINT0 de modo que cada vez que se entre un una
 * rutina de interrupcion externa el duty cycle incremente en un 10% (1mS). Esto
 * se repite hasta llegar al 100%, luego, si se entra nuevamente a la interrupcion
 * externa, el duty cycle volvera al 10& inicial
 * 2) Modificar los niveles de prioridad para que la interrupcion por SysTick tenga
 * mayor prioridad que la interrupcion externa
 * author: Agustin Vera
 * date 03/04/2026
 * version 1.0
 */

/*
entonces lo que hace el SysTick es configurar el PWM
PeriodoInt = (load+1) * frecClock
(1mS * 100MHz) - 1 = load
load = 99999
---------------
Duty = Th / T;
 0.1 * 10mS = Th
 Th = 1mS
 --------------
 El flujo del programa inicial seria:
 	 Habilito SysTick, SysTick interumpe cada XmS

 	 Interrumpe ---> Prendo Led y dejo pasar tiempo Y
 	 Pasa XmS
 	 Interrumpe ---> apago led, dejo pasar tiempo Y-X

 	 Interrumpe EINT0 ->> Pregunto si llegue al 100% o aumento (Th o X) (1mS)
 	 (10mS * 100MHz) - 1 = load, equivale al 100% del duty cycle
 	 load = 999999dec
*/
#define ThValor	0x1869F
volatile uint32_t Th = ThValor; // = 99999d
const uint32_t valorMaxTh =  0xF423F;// = 999999dec
volatile static uint32_t suma = ThValor;

void configGPIO();
void configSysTick(uint32_t);
void configEINTX();

int main(void){
	configGPIO();
	configEINTX();
	//luego de haber configurado pines e interrupciones, habilito SysTick inicial
	configSysTick(valorMaxTh);//dejo pasar 10mS

	while(1){

	}
}

void configGPIO(){
//P0.22 como salida
	LPC_PINCON->PINSEL1 &= ~(0b11 << 12);
	LPC_PINCON->PINMODE1 &= ~(0b11 << 12);
	//EINT0
	LPC_PINCON->PINSEL4 &= ~(1 << 21);
	LPC_PINCON->PINSEL4 |= (1<<20);
	LPC_PINCON->PINMODE4 &= ~(0b11<<20);//configurado como pullup

	//salida
	LPC_GPIO0->FIODIR |= (1<<22);

	//desenergizo
	LPC_GPIO0->FIOCLR = (1<<22);

}
void configEINTX(){
	//no limpio la bandera ya que solo puedo limpiar la bandera si se encuentra activo por flanco, por reset
	//esta activado por nivel
	LPC_SC->EXTMODE |= (1<<0);
	LPC_SC->EXTPOLAR &= ~(1<<0);//activo por flanco de bajada esto es debido a la pull up
	LPC_SC->EXTINT = (1<<0);
	NVIC_SetPriority(EINT0_IRQn,2);
	NVIC_SetPriority(SysTick_IRQn,1);//prioridad del systick
	NVIC_EnableIRQ(EINT0_IRQn);

}
void configSysTick(uint32_t x){
	//x se refiere al tiempo en alto o en bajo
	SysTick->LOAD = x;
	SysTick->VAL = 0;
	SysTick->CTRL = (0x7 << 0);

}

void SysTick_Handler(){
	if(~(LPC_GPIO0->FIOPIN) & (1<<22)){
		//si entro aqui significa que el pin esta apagado
		LPC_GPIO0->FIOSET = (1<<22);
		configSysTick(suma); //le paso como parametro a la funcion tiempo en alto
	}
	else
	{

		if(valorMaxTh == suma )
		{
			configSysTick(suma);
		}
		else{
			LPC_GPIO0->FIOCLR = (1<<22);
			configSysTick(valorMaxTh - suma); //le paso como parametro a la funcion tiempo en bajo

		}

	}
	SysTick->CTRL &= SysTick->CTRL;// coloco esta linea pq el profe pide
}
void EINT0_IRQHandler(void){


	if((LPC_SC->EXTINT) & (1<<0)){
		//interrupcion activa
		 if((Th + suma) <= valorMaxTh)
		 {
			 suma += Th;
		 }
		 else
		 {
			 suma = Th;
		 }

	}
	else{
		//ruido
	}
	LPC_SC->EXTINT = (1<<0); // LIMPIO BANDERA EINT0


}




















