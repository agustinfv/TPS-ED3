/*******************************************************************************//**
 * @file     dac.c
 * @brief    Implementación del driver de inicialización del DAC listo para DMA.
 * @details  Implementa DAC_Config() para dejar el DAC listo para recibir muestras
 *           desde el GPDMA de forma continua. El tono de salida es de
 *           440 Hz.
 * @note     ESW.2.1.13
 **********************************************************************************/

#include "dac.h"


/*******************************************************************************//**
 * @brief    Macro del contador de time out del DAC.
 * @details	 El cálculo se realizó de la siguiente manera:
 * 			 Tono de los buzzer elegido: 440 Hz.
 *  		 LUT de 32 muestras -> F = 440 * 32 = 14080 Hz.
 *  		 T = 1 / 14080 ≈ 71 µs.
 *
 *  		 El contador del DAC trabaja con PCLK_DAC = CCLK / 4 = 100 MHz / 4 = 25 MHz.
 *  		 COUNT_VALUE = 71 µs * 25 MHz ≈ 1775.
 **********************************************************************************/
#define COUNT_VALUE     1775

/*******************************************************************************//**
 * @brief    Inicializa el DAC listo para transferencia continua mediante el DMA.
 * @details  Configura P0.26 como AOUT, inicializa el DAC, habilita el contador
 * 			 interno de time out, las solicitudes DMA y carga COUNT_VALUE en el
 * 			 contador del DAC para reproducir la senoidal a 440 Hz.
 * @note     USW.2.1.13.1
 **********************************************************************************/
void DAC_Config(void)
{
	DAC_CONVERTER_CFG_Type cfgDAC;

	cfgDAC.DBLBUF_ENA = 0;
	cfgDAC.CNT_ENA = 1;
	cfgDAC.DMA_ENA = 1;

	PINSEL_SetPinFunc(PINSEL_PORT_0, PINSEL_PIN_26, PINSEL_FUNC_2);

	DAC_Init(LPC_DAC);

	DAC_ConfigDAConverterControl(LPC_DAC, &cfgDAC);
	DAC_SetDMATimeOut(LPC_DAC, COUNT_VALUE);
}
