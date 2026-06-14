/*******************************************************************************//**
 * @file     dac.h
 * @brief    Driver de inicialización del DAC listo para DMA.
 * @details  Configura el pin P0.26 como AOUT, inicializa el DAC y habilita las
 * 1		 solicitudes de DMA para la reproducción continua de la onda senoidal
 * 			 a 440 Hz.
 * @note     ESW.2.1.13
 **********************************************************************************/

#ifndef DAC_H
#define DAC_H

#include "lpc17xx_dac.h"
#include "gpio.h"

/*******************************************************************************//**
 * @brief    Inicializa el DAC listo para transferencia continua mediante el DMA.
 * @details  Configura P0.26 como AOUT mediante PINSEL_SetPinFunc(), inicializa el
 *           periférico con DAC_Init(), habilita el contador interno y las
 *           solicitudes DMA. Finalmente, carga el valor COUNT_VALUE (1775) en el
 *           contador del DAC con para lograr un tono de salida de 440 Hz con
 *           la senoidal.
 * @note     USW.2.1.13.1
 **********************************************************************************/
void DAC_Config(void);

#endif
