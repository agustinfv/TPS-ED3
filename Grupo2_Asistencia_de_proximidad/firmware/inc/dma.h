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
 * @param lut      Puntero constante al array con la señal en memoria Flash o RAM.
 * @param lutSize  Cantidad de muestras de la señal.
 **********************************************************************************/
void DMA_Config(const uint32_t *lut, uint32_t lutSize);

/*******************************************************************************//**
 * @brief    Actualiza la fuente de datos (LUT) y recarga el canal DMA.
 * @details  Actualiza la LLI para apuntar a la nueva tabla, recarga los registros
 *           del canal (SrcAddr, DstAddr, LLI, Control) desde la LLI y reactiva el
 *           DMA. Se utiliza para cambiar el volumen o sonido en tiempo real.
 * @note     USW.2.1.14.1
 *
 * @param lut  Puntero constante a la nueva tabla senoidal a reproducir.
 **********************************************************************************/
void DMA_Reload(const uint32_t *lut);

#endif
