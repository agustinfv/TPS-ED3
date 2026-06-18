/*******************************************************************************//**
 * @file     buzzer.h
 * @brief    Driver para la generación de los pitidos de los buzzers.
 * @details  Se encarga de dejar el hardware listo para reproducir la señal senoidal
 * 			 y modifica el volumen de esta cuando sea requerido utilizando tablas
 *           precalculadas.
 * @note     ESW.2.1.4 / 2.1.8
 **********************************************************************************/

#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include "dac.h"
#include "dma.h"
#include "timer.h"

/*******************************************************************************//**
 * @brief    Enum para identificar ambos Buzzer.
 **********************************************************************************/
typedef enum
{
    BUZZER_FRONT,
    BUZZER_REAR
} buzzer_id;

/*******************************************************************************//**
 * @brief    Enum que indica si el buzzer se silencia o suena.
 **********************************************************************************/
typedef enum
{
	MUTE,
	PLAY
} buzzer_action;

/*******************************************************************************//**
 * @brief    Inicializa el hardware necesario para la generación de la senoidal.
 * @details  Configura el Timer1 y el DAC y el DMA para la reproducción continua
 *			 de la señal.
 * @note     USW.2.1.4.1 / 2.1.8.1
 **********************************************************************************/
void Buzzer_Init(void);

/*******************************************************************************//**
 * @brief    Ajusta el volumen de la señal cambiando la tabla senoidal.
 * @details  De acuerdo al volumen indicado, selecciona la tabla (LUT) precalculada
 *           correspondiente a esa amplitud y recarga el DMA para que la comience a
 *           enviar al DAC sin necesidad de realizar procesamiento por CPU.
 * @note     USW.2.1.4.1 / 2.1.8.1
 *
 * @param volume  Nivel de volumen deseado (0 = silencio, 100 = volumen máximo).
 **********************************************************************************/
void Buzzer_ApplyVolume(uint8_t volume);

#endif
