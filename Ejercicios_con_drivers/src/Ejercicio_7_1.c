#include "LPC17xx.h"

/*
 * Generar una señãl analogica tipo rampa lineal de 0 a 3v3 a una frec
 * de 976Hz
 * Implementar base de tiempo para la generacion via timer 0
 * Implementar unicamente con drives del fabricante
 *
 * author Agustin Vera
 * date 29/04/2026
 */

/*
 * fsignal = fdelay/Nsample
 * Defino 1024 muestras
 * 1/ (976Hz * 1024) = tdelay
 * tdelay = 1uS (interrupcion cada 1us)
 * Observo que el tiempo de delay conside con el tiempo de settling del DAC
 */



#include "lpc17xx_pinsel.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_timer.h"

#define NRO_SAMPLE 1024

volatile uint16_t array[NRO_SAMPLE];
volatile static uint32_t indexWAVE = 0;


void configDAC();
void configTIMER0();
void waveFORM();

int main(void){
	waveFORM();
	configDAC();
	configTIMER0();



	while(1){

	}
}

void configDAC(){
	//Configuracion
	PINSEL_CFG_Type cfgPINDAC;

	//Seleccion
	cfgPINDAC.Portnum = PINSEL_PORT_0;
	cfgPINDAC.Pinnum = PINSEL_PIN_26;
	cfgPINDAC.Funcnum = PINSEL_FUNC_2;
	cfgPINDAC.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgPINDAC.OpenDrain = PINSEL_PINMODE_NORMAL;


	//Inicializacion
	PINSEL_ConfigPin(&cfgPINDAC); //termine de configurar P0.26 en modo DAC
	DAC_Init(LPC_DAC);//sete el maximo ratio de conversion en 1Mhz osea 1uS, y AOUT = 0

}

void configTIMER0(){
	//Declaracion variables de timer y match
	TIM_TIMERCFG_Type cfgTimer0;
	TIM_MATCHCFG_Type cfgMatch0;

	//Config timer
	cfgTimer0.PrescaleOption = TIM_PRESCALE_USVAL;//int en uS
	cfgTimer0.PrescaleValue = 1; //int cada 1uS, funcion init se encarga de restarle el -1

	//Config match0
	cfgMatch0.MatchChannel = 0;
	cfgMatch0.MatchValue = 1 - 1; //quiero la int cada 1uS
	cfgMatch0.IntOnMatch = ENABLE;
	cfgMatch0.StopOnMatch = DISABLE;
	cfgMatch0.ResetOnMatch = ENABLE;
	cfgMatch0.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;

	//Pre habilitacion timer
	TIM_Init(LPC_TIM0,TIM_TIMER_MODE,&cfgTimer0);
	TIM_ConfigMatch(LPC_TIM0,&cfgMatch0);
	TIM_Cmd(LPC_TIM0,ENABLE);

	//Habilitacion Timer0
	NVIC_EnableIRQ(TIMER0_IRQn);


}
void waveFORM(){

	for(volatile int i = 0; i < NRO_SAMPLE;i++)
	{
		array[i] = i;

	}
}

void TIMER0_IRQHandler(void){
	if(TIM_GetIntStatus(LPC_TIM0,TIM_MR0_INT))
	{
		DAC_UpdateValue(LPC_DAC,array[indexWAVE]);


		if(indexWAVE == (NRO_SAMPLE-1)){
			indexWAVE = 0;
		}
		else
		{
			indexWAVE++;
		}
	}
	TIM_ClearIntPending(LPC_TIM0,TIM_MR0_INT);
}






