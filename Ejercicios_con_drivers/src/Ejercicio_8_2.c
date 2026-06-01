/*
 * Realizar la generacion de una forma de onda desde un buffer especifico en la AHB SRAM
 * a travez del modulo DAC, de manera indefinida miediante una LLI circular. Monitoreando
 * el estado de la transferencia mediante interrupcion del GPDMA
 * Configuracion del periferico DAC para transferencias M2P con LLI simple e ISR via GPDMA
 *
 * author: Agustin Vera
 * date: 12/05/2026
 * version 1.0
 */

//Includes
#include "LPC17xx.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_pinsel.h"



#define NRO_MUESTRA 1000
#define startAdress 0x2007C000

volatile uint32_t* BufferStart = (uint32_t *) startAdress;


void configDAC();
void configDMA();
void generarOnda();

int main(void){
	generarOnda();//recordar que a la onda la debo guardar en la memoria 2007C000 en adelante
	configDAC();
	configDMA();

	GPDMA_ChannelCmd(0,ENABLE);//comienzo transmision
	while(1){

	}
}

void generarOnda(){
	volatile uint32_t *Punt_Memoria = (uint32_t *) startAdress;
	for(volatile int i = 0; i < NRO_MUESTRA;i++){
		if(i > ((NRO_MUESTRA/2)-1))
		{

			*Punt_Memoria = 0;
		}
		else{

			*Punt_Memoria = 10;
		}
		Punt_Memoria++;
	}


}


void configDAC(){
	DAC_CONVERTER_CFG_Type cfgDAC;
	PINSEL_CFG_Type cfgAOUT;
	uint32_t value = 65535;

	//DAC
	cfgDAC.DBLBUF_ENA = 0;
	cfgDAC.CNT_ENA = 1;
	cfgDAC.DMA_ENA = 1;
	//Pinsel
	cfgAOUT.Portnum = PINSEL_PORT_0;
	cfgAOUT.Pinnum = PINSEL_PIN_26;
	cfgAOUT.Funcnum = PINSEL_FUNC_2;
	cfgAOUT.Pinmode = PINSEL_PINMODE_TRISTATE;
	cfgAOUT.OpenDrain = PINSEL_PINMODE_NORMAL;


	PINSEL_ConfigPin(&cfgAOUT);

	DAC_Init(LPC_DAC);
	DAC_ConfigDAConverterControl(LPC_DAC,&cfgDAC);
	DAC_SetDMATimeOut(LPC_DAC,value);
}

void configDMA(){
	static GPDMA_LLI_Type cfgLLI;
	GPDMA_Channel_CFG_Type cfgChannel;

	GPDMA_Init();

	cfgChannel.ChannelNum = 0;
	cfgChannel.TransferSize = NRO_MUESTRA;
	cfgChannel.TransferWidth = 0;
	cfgChannel.SrcMemAddr = (uint32_t) BufferStart;
	cfgChannel.DstMemAddr = 0;
	cfgChannel.TransferType = GPDMA_TRANSFERTYPE_M2P;
	cfgChannel.SrcConn = 0;
	cfgChannel.DstConn = GPDMA_CONN_DAC;
	cfgChannel.DMALLI = (uint32_t)&cfgLLI; //hago que sea una lista circular

	cfgLLI.SrcAddr = (uint32_t) BufferStart;
	//mi direccion de destino seria el pin AOUT
	cfgLLI.DstAddr = (uint32_t) &LPC_DAC->DACR;
	cfgLLI.NextLLI = (uint32_t) &cfgLLI;
	cfgLLI.Control = (1<<31) | (0 << 27) | (0b1<<26) | (0b010<<21) | (0b010<<18) | (0b000000<<12) | (0x3E8<0);


	GPDMA_Setup(&cfgChannel);
	NVIC_EnableIRQ(DMA_IRQn);

}
