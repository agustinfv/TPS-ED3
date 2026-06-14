/*******************************************************************************//**
 * @file     dma.h
 * @brief    Driver del DMA para transferencia continua de la senoidal al DAC.
 * @details  Contiene DMA_Config() y DMA_Reload() para inicializar y actualizar el
 *           canal 0 del DMA del LPC1769. La transferencia se da en modo
 *           M2P usando una LLI circular que apunta a sí misma para reproducir
 *           la señal continuamente.
 *           Las muestras de la señal deben estar desplazadas << 6 lugares para
 *           alinear el valor de 10 bits con los bits del registro DACR.
 * @note     ESW.2.1.14
 **********************************************************************************/

#ifndef DMA_H
#define DMA_H

#include <stdint.h>
#include "lpc17xx_gpdma.h"
#include "dac.h"

/*******************************************************************************//**
 * @brief    Macro con el canal del DMA asignado al DAC.
 **********************************************************************************/
#define DMA_CHANNEL_DAC     0

/*******************************************************************************//**
 * @brief    Cantidad de muestras de la señal senoidal.
 **********************************************************************************/
#define DMA_LUT_SIZE    32

/*******************************************************************************//**
 * @brief    Inicializa el GPDMA y configura el canal 0 para enviar la señal al DAC
 *           de forma continua mediante una LLI circular.
 * @details  Inicializa el controlador GPDMA, configura la LLI para transferencia
 * 			 continua, y configura y activa el canal 0
 * @note     USW.2.1.14.1
 *
 * @param lut      Puntero al array con la señal (valores de 32 bits, << 6).
 * @param lutSize  Cantidad de muestras de la señal.
 **********************************************************************************/
void DMA_Config(uint32_t *lut, uint32_t lutSize);

#endif
