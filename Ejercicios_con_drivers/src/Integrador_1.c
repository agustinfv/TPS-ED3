/*
 * author Agustin Vera
 * version 1.0
 */

#include "LPC17xx.h"
#include "lpc17xx_clkpwr.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_timer.h"
#include <stdbool.h>

#define firstBank0 0x2007C000 //Primera mitad del banco 0
#define halfBank0 0x2007E000 //Segunda mitad del banco 0
#define bufferSize 4096


void configPIN();
void configADC();
void configPUNT();
void configTIMER();



static TIM_TIMERCFG_Type TIMpresc;
static TIM_MATCHCFG_Type TIMatch;
volatile static int flagBoton = 0;


int main(void){
	configPIN();




	while(1){

	}
}

void configPIN(){
	PINSEL_CFG_Type pin2_10;
	EXTI_InitTypeDef eint;

	pin2_10.Portnum = PINSEL_PORT_2;
	pin2_10.Pinnum = PINSEL_PIN_10;
	pin2_10.Funcnum = PINSEL_FUNC_1;
	pin2_10.Pinmode = PINSEL_PINMODE_PULLDOWN;
	pin2_10.OpenDrain = PINSEL_PINMODE_NORMAL;

	PINSEL_ConfigPin(&pin2_10);


	//activo interrupcion por EINT0

	eint.EXTI_Line = EXTI_EINT0;
	eint.EXTI_Mode = EXTI_MODE_EDGE_SENSITIVE;
	eint.EXTI_Polarity = EXTI_POLARITY_HIGH_ACTIVE_OR_RISING_EDGE;

	EXTI_Config(&eint);
	NVIC_EnableIRQ(EINT0_IRQn);

}

void EINT0_IRQHandler(void){

	if(LPC_SC->EXTINT & (1<<0))
	{
	//Se presiono boton por lo tanto debo decidir si realizo primera o seg secuencia

		TIM_Cmd(LPC_TIM0, DISABLE);//desactivo timer
		TIM_ResetCounter(LPC_TIM0);//reseteo contador


		if(flagBoton == 0){
			configADC();
			configTIMER();
		}

		flagBoton++;
		if((flagBoton%2) == 1){
			//realizo primera secuencia

			ADC_IntConfig(LPC_ADC, ADC_ADINT0, ENABLE);
			NVIC_EnableIRQ(ADC_IRQn);


		}
		else if((flagBoton%2) == 0){
			//realizo segunda secuencia
			ADC_IntConfig(LPC_ADC,ADC_ADINT0,DISABLE);
			NVIC_DisableIRQ(ADC_IRQn);

		}

		TIM_Cmd(LPC_TIM0, ENABLE);//vuelvo activar timer para que cuente bien el segundo
	}else{
		//Ruido

	}

	EXTI_ClearEXTIFlag(EXTI_EINT0);
}

void configADC(){
	//config P0.23 para la entrada de datos
	PINSEL_CFG_Type p0_23;

	p0_23.Portnum = PINSEL_PORT_0;
	p0_23.Pinnum = PINSEL_PIN_23;
	p0_23.Funcnum = PINSEL_FUNC_1;
	p0_23.Pinmode = PINSEL_PINMODE_TRISTATE;
	p0_23.OpenDrain = PINSEL_PINMODE_NORMAL;


	PINSEL_ConfigPin(&p0_23);
	ADC_Init(LPC_ADC,160000);
	ADC_BurstCmd(LPC_ADC,ENABLE);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_0,ENABLE);

}

void configTIMER(){
	//timer cada 1 seg, debe tener mas prioridad que los demas



	TIMpresc.PrescaleOption = TIM_PRESCALE_USVAL;//microsecond
	TIMpresc.PrescaleValue = 1000000;

	TIMatch.MatchChannel = 0;
	TIMatch.IntOnMatch = ENABLE;
	TIMatch.StopOnMatch = DISABLE;
	TIMatch.ResetOnMatch = ENABLE; //busco que matchee, interrumpa, reset y que no se pare
	TIMatch.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
	TIMatch.MatchValue = 1-1;//int cada 1 segc


//Comento a proposito las sig lineas
//}
//Esta funcion debe ser llamada cuando quiero guardar datos cada 1 seg
//void callTimer(){
	TIM_Init(LPC_TIM0,TIM_TIMER_MODE,&TIMpresc);
	TIM_ConfigMatch(LPC_TIM0,&TIMatch);
	TIM_ResetCounter(LPC_TIM0);

	NVIC_EnableIRQ(TIMER0_IRQn);
	NVIC_SetPriority(TIMER0_IRQn,0);

	TIM_Cmd(LPC_TIM0,ENABLE);
}

void TIMER0_IRQHandler(){
	if(TIM_GetIntStatus(LPC_TIM0,TIM_MR0_INT) == 1){
		//transfiero M2M a la segunda mitad del banco, saco promedio y envio por UART


	}
	TIM_ClearIntPending(LPC_TIM0, TIM_MR0_INT);
}

void ADC_IRQHandler(){
	static volatile uint16_t *ADC_Pointer = (uint16_t *) firstBank0;

	if(ADC_ChannelGetStatus(LPC_ADC,ADC_CHANNEL_0,1) == 1)
	{
		//se termino de leer un dato por ADC
		uint16_t valorDesf = ADC_ChannelGetData(LPC_ADC,ADC_CHANNEL_0);
		uint16_t valormV = (valorDesf * 3300)/ 4096;
		*ADC_Pointer = valormV;

		ADC_Pointer++;
		if(ADC_Pointer >= (uint16_t *) halfBank0){
			ADC_Pointer = (uint16_t *) firtstBank0;
		}

	}


}
void configDMAM2M(){
	static GPDMA_LLI_Type cfgLLIM2M;
	GPDMA_Channnel_CFG_Type cfgChannel7;

	cfgChannel7.ChannelNum = 0;
	cfgChannel7.TransferSize = BufferSize;
	//cfgChannel7.TransfWidth =
	cfgChannel7.SrcMmeAddr = firstBank0;
	cf



}











