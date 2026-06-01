/*
 * Mediante la utilización del microcontrolador LPC1769,
 *  implemente el software necesario  para cumplir con los siguientes requerimientos:
 *
1)Usar el hardware GPDMA para transferir datos en palabras de 2 bytes desde la primera
mitad de la SRAM del banco 1 hacia la segunda mitad de la SRAM del banco 0.

2) Una vez finalizada la transferencia, mediante la interrupción por cuenta terminal
 del GPDMA, se debe encender un LED indicador conectado a P0.22.

3)Determine usted Cuántos bytes fueron transferidos


Notas:
Buffer a transferir:
(Destino - Inicial) / tamaño de palabra 2 bytes
(0x20083FFF - 0x2008000)/ sizeof(uint16_t)
 *
 *
 */

#include "LPC17xx.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_pinsel.h"

void configDMA();
void configPIN();

#define MEM_START_B1 0x20080000
#define MEM_END_B1 0x20082000 // 8192 bytes a hexa -> 2000
#define BufferSize (MEM_END_B1 - MEM_START_B1 )/sizeof(uint16_t)
#define SECOND_HALF_MEM_B0 0x2007C000 + BufferSize //En teoria es 0x2007E000

volatile uint16_t *PUNT_START_B1 = (uint16_t *) MEM_START_B1;
volatile uint16_t *PUNT_START_SECOND_B0 = (uint16_t *) SECOND_HALF_MEM_B0;


int main(void){
	configPIN();configDMA();

	GPDMA_ChannelCmd(7,ENABLE);
	while(1){

	}
}

void configPIN(){
	PINSEL_CFG_Type P0_22;


	P0_22.Portnum = PINSEL_PORT_0;
	P0_22.Pinnum = PINSEL_PIN_22;
	P0_22.Funcnum = PINSEL_FUNC_0;
	P0_22.Pinmode = PINSEL_PINMODE_TRISTATE;
	P0_22.OpenDrain = PINSEL_PINMODE_NORMAL;

	PINSEL_ConfigPin(&P0_22);

	LPC_GPIO0->FIODIR |= (1<<22);
	LPC_GPIO0->FIOCLR = (1<<22);//limpio la salida por seguridad


}

void configDMA(){
	//debo usar transf M2M, asumo transferir desde la segunda
	//hasta el final y linked simple

	//Nota, en el casteo a la memoria, nose si utilizar uint16_t o uint16_t *

	GPDMA_Channel_CFG_Type cfgDMA;
	static GPDMA_LLI_Type cfgLLI;

	NVIC_DisableIRQ(DMA_IRQn);
	GPDMA_Init();

	cfgLLI.SrcAddr = (uint32_t ) PUNT_START_B1;
	cfgLLI.DstAddr = (uint32_t ) PUNT_START_SECOND_B0;
	cfgLLI.NextLLI = 0; //transf y ya
	cfgLLI.Control = (cfgLLI.Control &= ~(0xFFFF << 0)) | (1<<31) | (1<<27) | (1<<26)
				| (1<<21) | (1<<18) | (BufferSize -1 <<0);
		//al SBSize y DBASize nose si configurarlo con el valor 1 o 16(0b011)



	cfgDMA.ChannelNum = 7;
	cfgDMA.TransferSize = BufferSize;
	cfgDMA.TransferWidth = sizeof(uint16_t);
	cfgDMA.SrcMemAddr = (uint32_t ) PUNT_START_B1;
	cfgDMA.DstMemAddr = (uint32_t ) PUNT_START_SECOND_B0;
	cfgDMA.TransferType = GPDMA_TRANSFERTYPE_M2M;
	cfgDMA.SrcConn = 0;//uso M2M
	cfgDMA.DstConn = 0;
	cfgDMA.DMALLI = (uint32_t)&cfgLLI;

	GPDMA_Setup(&cfgDMA);
	NVIC_EnableIRQ(DMA_IRQn);







}

void DMA_IRQHandler(void){
	if((GPDMA_IntGetStatus(GPDMA_STAT_INTTC,7)) == SET)
	{
		LPC_GPIO0->FIOSET = (1<<22);
	}

	GPDMA_ClearIntPending(GPDMA_STAT_INTTC,7);
}









