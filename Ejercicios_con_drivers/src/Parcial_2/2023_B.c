#include "LPC17xx.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_adc.h"

#define FIRST_HALF_B1 0x20080000
#define SECOND_HALF_B1 0x20082000 // 8192 bytes a hexa -> 2000

#define FIRST_HALF_B0 0x2007C000
#define SECOND_HALF_B0 0x2007E000
#define BufferSize (SECOND_HALF_B0 - FIRST_HALF_B0 )/sizeof(uint16_t)

#define SAMPLES 1024

typedef enum{
	CERO,
	FIRST
}STATES;

volatile uint16_t *PUNT_FIRST_HALF_B0 = (uint16_t *) FIRST_HALF_B0;
volatile uint16_t *PUNT_SECOND_HALF_B0 = (uint16_t *) SECOND_HALF_B0;
volatile STATES state = CERO;
volatile uint16_t wave_form[SAMPLES];
volatile uint16_t indice_array = 0;

void configDMA();
void configPIN();
void configADC();
void configDAC();


int main(void){
	//configs
	configPIN();
	configDMA();
	configADC();
	generarWAVE();
	while(1){
		switch(state){
		case 0:
			//solo debo reproducir la wave_form por el DAC

		}
		case 1:
			//saco muestras por el DAC atravez del DMA

	}
}

void generarWAVE(){
	//Debe estar almacenada en la segunda mitad del banco 0
	//Mi onda es una sierra tanto en el eje y > 0 y y < 0
	for(int i = 0; i < (SAMPLES/2);i++){
		wave_form[i]++;
	}
	for(int i = SAMPLES/2; i < SAMPLES;i++){
		wave_form[i] = -wave_form[i - 1];
	}
}


void configPIN(){
	//config P1_31 canal 5 del ADC, lo dejo de manera abstracta lo hare despues
	//config EXTINT

}

void configDAC(){
	//PCLK_DAC = 20Mhz
	//Me dice que mi señal tiene un periodo de 614uS por lo tanto uso la formula
	//Value = pclk_dac/1/614e-6 = 12280

		uint32_t time_value = 12280;
		DAC_CONVERTER_CFG_Type dac;

		dac.DBLBUF_ENA = SET;
		dac.CNT_ENA = SET;
		dac.DMA_ENA = SET;

		DAC_Init(LPC_DAC); // inicia el bit de bias en 0 , su testablecimiento es 1uS
		DAC_ConfigDAConverterControl(LPC_DAC,&dac);
		DAC_SetDMATimeOut(LPC_DAC,time_value);






}

void configADC(){
	ADC_Init(LPC_ADC,32000);
	ADC_BurstCmd(LPC_ADC,ENABLE);
	ADC_ChannelCmd(LPC_ADC,ADC_CHANNEL_5,ENABLE);
}


void configDMA(){

		//------TRANSFERENCIAS P2M------
		static GPDMA_LLI_Type LLI_ADC;
		GPDMA_Channel_CFG_Type ADC_DMA;

		NVIC_DisableIRQ(DMA_IRQn);
		GPDMA_Init();

		LLI_ADC.SrcAddr = (uint32_t) & (LPC_ADC->ADDR5);
		LLI_ADC.DstAddr = (uint32_t) PUNT_FIRST_HALF_B0;
		LLI_ADC.NextLLI = (uint32_t)&LLI_ADC;
		LLI_ADC.Control =  (1<<31)|(1<<27)|(1<<21)|(1<<18)|	(BufferSize-1 << 0);

		ADC_DMA.ChannelNum = 0;
		ADC_DMA.TransferSize = BufferSize;
		ADC_DMA.TransferWidth = 0;
		ADC_DMA.SrcMemAddr = 0;
		ADC_DMA.DstMemAddr = (uint32_t) PUNT_FIRST_HALF_B0;
		ADC_DMA.TransferType = GPDMA_TRANSFERTYPE_P2M;
		ADC_DMA.SrcConn = GPDMA_CONN_ADC;
		ADC_DMA.DstConn = 0;
		ADC_DMA.DMALLI = (uint32_t) &LLI_ADC;


		if(GPDMA_Setup(&ADC_DMA) == SUCCESS){
			//CANAL CONFIG CORRECTAMENTE
		}
		else{
			//el canal no se pudo configurar
		}
		//NVIC_EnableIRQ(DMA_IRQn);
		GPDMA_ChannelCmd(0,ENABLE);


		// -------TRANSFERENCIAS M2P (SECOND_HALF_B0_MEM->DAC)----------
		static GPDMA_LLI_Type LLI_DAC;
		GPDMA_Channel_CFG_Type DAC_DMA;

		NVIC_DisableIRQ(DMA_IRQn);
		GPDMA_Init();

		LLI_DAC.SrcAddr = (uint32_t) PUNT_SECOND_HALF_B0; //Segunda
		LLI_DAC.DstAddr = (uint32_t) &LPC_DAC->DACR;
		LLI_DAC.NextLLI = (uint32_t)&LLI_DAC;
		LLI_DAC.Control = (1<<31) | (1<<26) | (1<<21) | (1<<18) | (BufferSize-1 << 0);

		DAC_DMA.ChannelNum = 1;
		DAC_DMA.TransferSize = BufferSize;
		DAC_DMA.TransferWidth = 0;
		DAC_DMA.SrcMemAddr = (uint32_t) PUNT_SECOND_HALF_B0;
		DAC_DMA.DstMemAddr = 0;
		DAC_DMA.TransferType = GPDMA_TRANSFERTYPE_M2P;
		DAC_DMA.SrcConn = 0;
		DAC_DMA.DstConn = GPDMA_CONN_DAC;
		DAC_DMA.DMALLI = (uint32_t) &LLI_DAC;

		if(GPDMA_Setup(&DAC_DMA) == SUCCESS){
		//CANAL CONFIG CORRECTAMENTE
		}
		else{
			//el canal no se pudo configurar
		}
		//NVIC_EnableIRQ(DMA_IRQn);
		GPDMA_ChannelCmd(0,DISABLE);




}



void EINT0_IRQHandler(){

	switch(state){
		case 0:

			ADC_BurstCmd(LPC_ADC,DISABLE);
			GPDMA_ChannelCmd(0,DISABLE);//P2M
			GPDMA_ChannelCmd(1,ENABLE);//M2P
			state = FIRST;

			break;
		case 1:
			ADC_BurstCmd(LPC_ADC,ENABLE);
			GPDMA_ChannelCmd(0,ENABLE);//P2M
			GPDMA_ChannelCmd(1,DISABLE);//M2P
			state = CERO;
			break;


	}

}
