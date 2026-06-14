/*******************************************************************************//**
 * @file     dma.c
 * @brief    Implementación del driver DMA para reproducción continua de la señal.
 * @details  Configura el canal 0 del DMA en modo M2P con una LLI circular que
 * 			 permite reproducir la senoidal indefinidament.
 * @note     ESW.2.1.14
 **********************************************************************************/

#include "dma.h"

/*******************************************************************************//**
 * @brief    Macro con la configuración del campo control de una LLI.
 * @details	 La configuración se realiza de la siguiente manera:
 * 			 Bits 11-0  -> Transfer size.
 *  		 Bits 20-18 -> Source width:  10 -> palabra de 32 bits.
 *  		 Bits 23-21 -> Dest width:    10 -> palabra de 32 bits.
 * 		 	 Bit  26    -> Source increment: 1 -> Para recorrer la señal.
 *  		 Bit  27    -> Dest increment:   0 -> Para siempre mandar datos al DAC.
 **********************************************************************************/
#define DMA_CONTROL(size)  					  \
				( ((size) & 0xFFF)    |       \
					(2 << 18)         |       \
					(2 << 21)         |       \
					(1 << 26)         )

/*******************************************************************************//**
 * @brief    Variable de configuración de la LLI para el DMA.
 **********************************************************************************/
static GPDMA_LLI_Type dacLLI;

/*******************************************************************************//**
 * @brief    Inicializa el DMA y configura el canal 0 para enviar la señal al DAC
 *           de forma continua mediante una LLI circular.
 * @details  Inicializa el DMA con GPDMA_Init(). Construye la LLI con la
 *           dirección fuente de la señal, destino (DACR), NextLLI apuntando a
 *           sí misma y la parte control ya definida, con DMA_CONTROL(size).
 *           Completa el struct GPDMA_Channel_CFG_Type y activa el canal con
 *           GPDMA_Setup() y GPDMA_ChannelCmd().
 * @note     USW.2.1.14.1
 *
 * @param lut      Puntero al array con la señal (valores de 32 bits, << 6).
 * @param lutSize  Cantidad de muestras de la señal.
 **********************************************************************************/
void DMA_Config(uint32_t *lut, uint32_t size)
{
	GPDMA_Channel_CFG_Type dmaCfg;

	GPDMA_Init();

	dacLLI.SrcAddr = (uint32_t)lut;
	dacLLI.DstAddr = (uint32_t)(&LPC_DAC->DACR);
	dacLLI.NextLLI = (uint32_t)(&dacLLI);
	dacLLI.Control = DMA_CONTROL(size);

	dmaCfg.ChannelNum    = DMA_CHANNEL_DAC;
	dmaCfg.SrcMemAddr    = (uint32_t)lut;
	dmaCfg.DstMemAddr    = (uint32_t)(&LPC_DAC->DACR);
	dmaCfg.TransferSize  = size;
	dmaCfg.TransferWidth = GPDMA_WIDTH_WORD;
	dmaCfg.TransferType  = GPDMA_TRANSFERTYPE_M2P;
	dmaCfg.SrcConn       = 0;
	dmaCfg.DstConn       = GPDMA_CONN_DAC;
	dmaCfg.DMALLI        = (uint32_t)(&dacLLI);

	GPDMA_Setup(&dmaCfg);
	GPDMA_ChannelCmd(DMA_CHANNEL_DAC, ENABLE);
}
