/*******************************************************************************//**
 * @file     transistor.h
 * @brief    Driver de hardware para los transistores NPN de los buzzers.
 * @details  Contiene la configuración y el control de los pines GPIO conectados
 *           a la base de los transistores NPN que habilitan o cortan el flujo de
 *           audio hacia los buzzers.
 * @note     ESW.2.1.5 / 2.1.9
 **********************************************************************************/

#ifndef TRANSISTOR_H
#define TRANSISTOR_H

#include <stdint.h>
#include "gpio.h"
#include "sound_ctrl.h"

/*******************************************************************************//**
 * @brief    Macro con los puertos y pines correspondientes para cada sensor.
 * @details  La lógica de control de estos es la siguiente:
 *      	 GPIO = 0 -> base sin tensión -> transistor en corte  -> buzzer suena.
 *    		 GPIO = 1 -> base con tensión -> transistor saturado  -> buzzer en silencio.
 **********************************************************************************/
#define TRANSISTOR_FRONT_PORT   0
#define TRANSISTOR_FRONT_PIN    4
#define TRANSISTOR_REAR_PORT    0
#define TRANSISTOR_REAR_PIN     5

/*******************************************************************************//**
 * @brief    Configura los GPIO de los transistores como salidas en alto.
 * @details  Inicializa de la base de los transistores como salidas GPIO y los pone
 * 			 en estado alto para que los buzzer comiencen en silencio.
 * @note     USW.2.1.5.1 / 2.1.9.1
 **********************************************************************************/
void Transistor_Init(void);

/*******************************************************************************//**
 * @brief    Aplica un estado de encendido o apagado al transistor del buzzer.
 * @details  Controla el GPIO correspondiente:
 *           PLAY -> transistor en corte -> buzzer suena.
 *           MUTE -> transistor saturado -> silencio.
 * @note     USW.2.1.5.1 / 2.1.9.1
 *
 * @param id      BUZZER_FRONT o BUZZER_REAR.
 * @param action  PLAY habilitar sonido, MUTE para silenciar.
 **********************************************************************************/
void Transistor_SetState(buzzer_id id, buzzer_action action);

#endif
