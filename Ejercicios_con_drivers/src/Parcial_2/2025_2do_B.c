/*
 * Por un pin del ADC del microcontrolador LPC1769 ingresa una tensión de rango dinámico 0 a 3,3[v]
 * proveniente de un sensor de temperatura. Debido a la baja tasa de variación de la señal,
 * se pide tomar una muestra cada 30[s].
 * Pasados los 2[min] se debe promediar las últimas 4 muestras y en función de este valor,
 * tomar una decisión sobre una salida digital de la placa:
 *
 * Voy a realizar el ejercicio completo sin DMA.
 *
Si el valor es <1 [V] colocar la salida en 0 (0[V]).
Si el valor es >= 1[V] y <=2[V] sacar una señal intermitente de  F= ????
Si el valor es > 2[V] colocar la salida en 1 (3,3[V]).

CCLK = 100MHz
 *
 *
 */


#include "LPC17xx.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"

#define ONE_V 1000
#define TWO_V 2000

void configADC();
void configTIMER();
void configPIN();

int main(void){
	configPIN();
	configADC();
	configTIMER();

}

void configPIN(){
	//como el ejercicio no me explicita, asumo por mi cuenta
	//PINSEL_CFG_Type MAT0; No lo uso ya que es interrupcion interna
	PINSEL_CFG_Type ADC;
	/*
	MAT0.Portnum = PINSEL_PORT_0;
	MAT0.Pinnum = PINSEL_PIN_29;
	MAT0.Funcnum = PINSEL_FUNC_3;
	MAT0.Pinmode = PINSEL_PINMODE_TRISTATE;
	MAT0.OpenDrain = PINSEL_PINMODE_NORMAL;
	*/
	ADC.Portnum = PINSEL_PORT_0;
	ADC.Pinnum = PINSEL_PIN_23;
	ADC.Funcnum = PINSEL_FUNC_1;
	ADC.Pinmode = PINSEL_PINMODE_TRISTATE;
	ADC.OpenDrain = PINSEL_PINMODE_NORMAL;

	//PINSEL_ConfigPin(&MAT0);
	PINSEL_ConfigPin(&ADC);




}


void configADC(){
	ADC_Init(LPC_ADC,200000);
	ADC_BurstCmd(LPC_ADC,DISABLE);

	ADC_StartCmd(LPC_ADC,ADC_START_ON_MAT01);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_0,ENABLE);

	ADC_EdgeStartConfig(LPC_ADC,ADC_START_ON_RISING);
	ADC_IntConfig(LPC_ADC,ADC_ADINTEN0,ENABLE);
	NVIC_EnableIRQ(ADC_IRQn);

}

void configTIMER(){
	TIM_TIMERCFG_Type tC;
	TIM_MATCHCFG_Type tM;

	tC.PrescaleOption = TIM_PRESCALE_USVAL;
	tC.PrescaleValue = 15000000;//1e6 = 1 segundo, QUIERO LEER DATO CADA 30seg
	// por lo tanto mi segundo flanco de subida se produce prescValue * 2

	tM.MatchChannel = 1;
	tM.IntOnMatch = DISABLE;
	tM.StopOnMatch = DISABLE;
	tM.ResetOnMatch = ENABLE;
	tM.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
	tM.MatchValue = 0;

	TIM_Init(LPC_TIM0,TIM_TIMER_MODE,&tC);
	TIM_ConfigMatch(LPC_TIM0,&tM);
	TIM_Cmd(LPC_TIM0, ENABLE);
	//NVIC_EnableIRQ(TIMER0_IRQn);

}

/*

 Lee el dato adquirido por el ADC en el canal 0. La adquision del dato comenzo
  cuando ocurre el flanco de subida el MAT0.1, es decir interrumpicionTimer * 2



*/
void ADC_IRQHandler(void){

	//conozco que entra a esta funcion cada 30 seg,
	static volatile uint16_t indice;
	static volatile uint16_t array[4];

	if(ADC_ChannelGetStatus(LPC_ADC,ADC_CHANNEL_0) == SET)
	{
		array[indice] = (ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_0) * 3300)/4096; //valor guardado en mV
		indice = ( indice+ 1) % 4;
		if(indice == RESET){
			uint32_t promedio=0;
			for(volatile int i = 0; i <= 3;i++){
				promedio += array[i];
				//no hace falta limpiar al promedio ya que se borra de la memoria al salir del alcance de la funcion
			}
			promedio = promedio/4;

			if(promedio < ONE_V) //ONE_V = 1000
			{
				//coloco salida 0
			}else if (promedio > TWO_V){
				//coloco salida en 1
			}else{
				//saco una señal intermitente
			}

		}
	}
	//no encuentro una funcion que me indique que limpia la bandera de DONE del ADC

}

