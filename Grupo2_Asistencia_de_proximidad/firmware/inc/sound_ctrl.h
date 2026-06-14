/*******************************************************************************//**
 * @file     sound_ctrl.h
 * @brief    Controlador de la lógica del sistema de audio.
 * @details  Maneja la lógica de los sonidos: intercalado, volumen, y
 *           modos de pitido.
 * @note     ESW.2.1.5
 **********************************************************************************/

#ifndef SOUND_CTRL_H
#define SOUND_CTRL_H

#include "buzzer.h"

/*******************************************************************************//**
 * @brief    Macros que delimitan el volumen según los estados del sistema.
 **********************************************************************************/
#define VOL_SAFE			0
#define VOL_WARNING_LOW     25
#define VOL_WARNING_HIGH    50
#define VOL_DANGER_LOW      75
#define VOL_DANGER_HIGH     100

/*******************************************************************************//**
 * @brief    Macros que delimitan la frecuencia de los pitidos.
 * @details	 Un pitido se compone de un ciclo de sonido y uno de silencio.
 *  	 	 El threshold define la mitad del período: cuando el contador interno
 *  		 del buzzer lo alcanza, se hace toggle al transistor. Es por ello que
 *  		 el periodo completo de los pitidos es el doble de lo especificado
 *  		 debajo (la frecuencia mostrada ya lo contempló).
 *  		 Teniendo en cuenta que la base de tiempo es 1 ms:
 * 			 THRESHOLD_WARNING_LOW  -> 2 Hz.
 * 			 THRESHOLD_WARNING_HIGH -> 4 Hz.
 * 			 THRESHOLD_DANGER_LOW	-> aproximadamente 8 Hz.
 **********************************************************************************/
#define THRESHOLD_WARNING_LOW   250
#define THRESHOLD_WARNING_HIGH  125
#define THRESHOLD_DANGER_LOW     62

/*******************************************************************************//**
 * @brief    Enum que indica si el buzzer se silencia o suena.
 **********************************************************************************/

typedef enum
{
	MUTE,
	PLAY
}buzzer_action;

/*******************************************************************************//**
 * @brief    Modos de los pitidos.
 **********************************************************************************/
typedef enum
{
    BUZZER_MODE_OFF,
    BUZZER_MODE_BEEP,
    BUZZER_MODE_CONTINUOUS
} buzzer_mode;

/*******************************************************************************//**
 * @brief    Structs con los datos relevantes para un buzzer.
 **********************************************************************************/
typedef struct
{
    buzzer_id   	id;
    buzzer_mode 	mode;
    buzzer_action   action;
    uint32_t    	threshold;
    uint32_t    	counter;
    uint8_t     	volume;
} buzzer_t;

/*******************************************************************************//**
 * @brief    Declaramos los structs globales para acceso desde otros módulos.
 **********************************************************************************/
extern buzzer_t buzzer_front;
extern buzzer_t buzzer_rear;

/*******************************************************************************//**
 * @brief    Inicializa los estados lógicos de ambos buzzers.
 * @note     USW.2.1.3.1
 **********************************************************************************/
void SC_Init(void);

/*******************************************************************************//**
 * @brief    Establece el modo del pitido del buzzer indicado.
 * @note     USW.2.1.3.1
 *
 * @param id    BUZZER_FRONT o BUZZER_REAR.
 * @param mode  BUZZER_MODE_OFF, BUZZER_MODE_BEEP o BUZZER_MODE_CONTINUOUS.
 **********************************************************************************/
void SC_SetMode(buzzer_id id, buzzer_mode mode);

/*******************************************************************************//**
 * @brief    Asigna el volumen al buzzer indicado (0–100).
 * @note     USW.2.1.3.1
 *
 * @param id      BUZZER_FRONT o BUZZER_REAR.
 * @param volume  Nivel de volumen.
 **********************************************************************************/
void SC_SetVolume(buzzer_id id, uint8_t volume);

/*******************************************************************************//**
 * @brief    Actualiza el threshold del buzzer indicado.
 * @note     USW.2.1.3.1
 *
 * @param id         BUZZER_FRONT o BUZZER_REAR.
 * @param threshold  Semiperíodo en ms. Debe ser mayor a 0.
 **********************************************************************************/
void SC_SetThreshold(buzzer_id id, uint32_t threshold);

/*******************************************************************************//**
 * @brief    Se encarga de mostrar el estado lógico de ambos buzzers.
 * @details	 De acuerdo al estado lógico de cada buzzer, se emite el correspondiente
 *			 pitido y volumen de la señal. En el caso de ambos tener que sonar
 *			 juntos, se utiliza una lógica de intercalado que le brinda a cada
 *			 buzzer la posibilidad de emitir 3 pitidos por turnos, con su respectivo
 *			 volumen y tipo de pitido.
 * @note     USW.2.1.3.1
 **********************************************************************************/
void SC_ShowState(void);

#endif
