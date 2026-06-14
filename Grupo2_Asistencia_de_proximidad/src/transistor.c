/*******************************************************************************//**
 * @file     transistor.c
 * @brief    Implementación del driver de los transistores NPN.
 * @details  Maneja físicamente los pines GPIO de los transistores que actúan como
 *           llaves de encendido/apagado para el sistema de sonido (suena o no suena).
 * @note     ESW.2.1.5 / 2.1.9
 **********************************************************************************/

#include "transistor.h"

/*******************************************************************************//**
 * @brief    Configura los GPIO de los transistores como salidas en alto (silencio).
 * @details  Inicializa los pines P0.4 y P0.5 como salida. Los
 *           pone en nivel alto mediante GPIO_SetValue() para saturar los NPN y
 *           mantener los buzzer en silencio.
 * @note     USW.2.1.5.1 / 2.1.9.1
 **********************************************************************************/
void Transistor_Init(void)
{
    GPIO_ConfigPin(TRANSISTOR_FRONT_PORT, TRANSISTOR_FRONT_PIN, 1);
    GPIO_ConfigPin(TRANSISTOR_REAR_PORT,  TRANSISTOR_REAR_PIN,  1);
    
    GPIO_SetValue(TRANSISTOR_FRONT_PORT, (1 << TRANSISTOR_FRONT_PIN));
    GPIO_SetValue(TRANSISTOR_REAR_PORT,  (1 << TRANSISTOR_REAR_PIN));
}

/*******************************************************************************//**
 * @brief    Aplica un estado lógico al transistor del buzzer especificado.
 * @details  pone en 0 el pin GPIO correspondiente para habilitar el sonido
 * 			 (transistor en corte) o lo pone en 1 para silenciarlo (transistor
 * 			 saturado).
 * @note     USW.2.1.5.1 / 2.1.9.1
 *
 * @param id      BUZZER_FRONT o BUZZER_REAR.
 * @param action  PLAY para habilitar sonido, MUTE para silenciar.
 **********************************************************************************/
void Transistor_SetState(buzzer_id id, buzzer_action action)
{
    if(id == BUZZER_FRONT)
    {
        if(action) GPIO_ClearValue(TRANSISTOR_FRONT_PORT, (1 << TRANSISTOR_FRONT_PIN));
        else GPIO_SetValue(TRANSISTOR_FRONT_PORT,   (1 << TRANSISTOR_FRONT_PIN));
    }
    else
    {
        if(action) GPIO_ClearValue(TRANSISTOR_REAR_PORT, (1 << TRANSISTOR_REAR_PIN));
        else GPIO_SetValue(TRANSISTOR_REAR_PORT,   (1 << TRANSISTOR_REAR_PIN));
    }
}
