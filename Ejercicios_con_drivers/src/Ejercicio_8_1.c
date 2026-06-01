/*
 * Realiza la transferencia de datos adquiridos por el modulo ADC, desde el registro ADDR0
 * a un buffer especifico en la AHP SRAM, de manera indefinida mediante una LLI circular
 * Configuracion del periferico ADC para transferencias P2M con LLI simple
 *
 * author Agustin Vera
 * date 6/05/2026
 * version 1.0
 *
 */

//Includes
#include <stdio.h>
#include <stdbool.h>
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"

#define ADC_START 0X2007C000

//var globales
volatile uint32_t* Punt_ADC_Start = (uint32_t *)ADC_START;
volatile bool bandera = true;
volatile bool datosListos = false;


void configADC();
void configDMA();

int main(void){
	configADC();
	configDMA();

	while(1){
		if(bandera){
			//IniciarTransf();
			bandera = false;
		}
		if(datosListos){
			volatile uint32_t * Punt_Mostrar_Datos = (uint32_t *)ADC_START;
			for(volatile int i = 0; i < 100; i++){

			//printf('%lu\n', *(Punt_Mostrar_Datos+i));//datos crudo
			uint32_t valorDesf = *(Punt_Mostrar_Datos + 1);
			uint32_t valorADC = (valorDesf >> 4) & 0xFFF;
			uint32_t valormV = (valorADC*3300)/4096;
			printf('Valor ADC en mV: %lu\n', valormV);

			}
			datosListos = false;
		}

	}
}

void configADC(){
	//Declaro var a usar, voy a usar el pin P0.26
	PINSEL_CFG_Type cfgPINADC;


	//Configro el pin
	cfgPINADC.Portnum = PINSEL_PORT_0;
	cfgPINADC.Pinnum = PINSEL_PIN_26;
	cfgPINADC.Funcnum = PINSEL_FUNC_1;
	cfgPINADC.Pinmode =	PINSEL_PINMODE_TRISTATE;
	cfgPINADC.OpenDrain = PINSEL_PINMODE_NORMAL;

	//Inicializo PIN y ADC
	PINSEL_ConfigPin(&cfgPINADC);

	ADC_Init(LPC_ADC,200000);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_3,ENABLE);
	//ADC_StartCmd(LPC_ADC,ADC_START_STOP);
	ADC_BurstCmd(LPC_ADC,ENABLE);
	//ADC_IntConfig(LPC_ADC, ADC_ADINT3,ENABLE);
	//NVIC_EnableIRQ(ADC_IRQn); //No se habilita porque uso DMA

}

void configDMA(){
	GPDMA_Channel_CFG_Type cfgCanal;
	static GPDMA_LLI_Type cfgLLI;

	GPDMA_Init();
	cfgCanal.ChannelNum = PARAM_GPDMA_CHANNEL(0);
	cfgCanal.TransferType = GPDMA_TRANSFERTYPE_P2M;
	cfgCanal.SrcConn = GPDMA_CONN_ADC;
	//ver, DMALLI-> uso la direccion de memoria de la structura LLI
	cfgCanal.DMALLI = (uint32_t)&cfgLLI;// esto me permite hacer la lista circular
	cfgCanal.DstMemAddr = (uint32_t)Punt_ADC_Start;//LE BRINDO ESA DIRECCION PARA USAR AHB
	cfgCanal.TransferSize = 100; //asumo que son 100 muestras las que tomare

	cfgLLI.SrcAddr = (uint32_t) &(LPC_ADC->ADDR3); //selecciono canal 3, ya que en el pin P0.26 me estan ingresando los datos del ADC
	cfgLLI.DstAddr = (uint32_t)Punt_ADC_Start;//direccion de destino
	cfgLLI.NextLLI = (uint32_t) &(cfgLLI);//me pide LLI circular, entonces
	cfgLLI.Control = 0x20480064;// cfgLLI.Control = 1000000010010000000000001100100

	GPDMA_Setup(&cfgCanal);
	NVIC_EnableIRQ(DMA_IRQn);
}


void DMA_IRQHandler(void){
	if(GPDMA_IntGetStatus(GPDMA_STAT_INTTC,0) == 1)
	{
		//llamo a la funcion de mostrar datos leidos, los muestro en la consola
		datosListos = true;

	}

	GPDMA_ClearIntPending(GPDMA_STATCLR_INTTC,0);
}






