/*******************************************************************************//**
 * @file     sound_ctrl.c
 * @brief    Implementación del controlador de audio del sistema.
 * @details  Implementa la lógica de intercalado, pitidos e inicialización lógica,
 *           utilizando los drivers de transistor.h y buzzer.h para aplicar los
 *           cambios en el hardware (transistores y DAC).
 * @note     ESW.2.1.5
 **********************************************************************************/

#include "sound_ctrl.h"
#include "transistor.h"

/*******************************************************************************//**
 * @brief    Variables con los structs de los buzzer para representar a cada uno.
 **********************************************************************************/
buzzer_t buzzer_front;
buzzer_t buzzer_rear;

/*******************************************************************************//**
 * @brief    Inicializa los estados lógicos de ambos buzzers.
 * @details  Carga valores por defecto: buzzers apagados, pitido de 2 Hz, volumen
 * 			 nulo, etc.
 * @note     USW.2.1.5.7
 **********************************************************************************/
void SC_Init(void)
{
    buzzer_front.id          = BUZZER_FRONT;
    buzzer_front.mode        = BUZZER_MODE_OFF;
    buzzer_front.action 	 = MUTE;
    buzzer_front.threshold   = 250;
    buzzer_front.counter     = 0;
    buzzer_front.volume      = 0;

    buzzer_rear.id           = BUZZER_REAR;
    buzzer_rear.mode         = BUZZER_MODE_OFF;
    buzzer_rear.action  	 = MUTE;
    buzzer_rear.threshold    = 250;
    buzzer_rear.counter      = 0;
    buzzer_rear.volume       = 0;
}

/*******************************************************************************//**
 * @brief    Establece el modo del pitido del buzzer indicado.
 * @note     USW.2.1.3.1
 **********************************************************************************/
void SC_SetMode(buzzer_id id, buzzer_mode mode)
{
    if(id == BUZZER_FRONT) buzzer_front.mode = mode;
    else buzzer_rear.mode = mode;
}

/*******************************************************************************//**
 * @brief    Asigna el volumen al buzzer indicado (0–100).
 * @note     USW.2.1.3.1
 **********************************************************************************/
void SC_SetVolume(buzzer_id id, uint8_t volume)
{
    if(volume > 100) volume = 100;

    if(id == BUZZER_FRONT) buzzer_front.volume = volume;
    else buzzer_rear.volume  = volume;
}

/*******************************************************************************//**
 * @brief    Actualiza el threshold del buzzer indicado.
 * @note     USW.2.1.3.1
 **********************************************************************************/
void SC_SetThreshold(buzzer_id id, uint32_t threshold)
{
    if(threshold == 0) return;

    if(id == BUZZER_FRONT) buzzer_front.threshold = threshold;
    else buzzer_rear.threshold  = threshold;
}

/*******************************************************************************//**
 * @brief    Se encarga de mostrar el estado lógico de ambos buzzers.
 * @details  De acuerdo al estado en el que se encuentre cada buzzer, emite o no el
 *			 pitido correspondiente. Dada la limitación se un solo DAC, se
 *			 delimitan los buzzer activos y en base a esto se dan 3 casos:
 *			 Caso A: Sensor frontal activo, opera normalmente según su estado.
 *			 Caso B: Sensor trasero activo, opera normalmente según su estado.
 *			 Caso C: Sensor frontal y trasero activos. Se utiliza una lógica de
 *			 intercalado para permitir que cada buzzer emita 3 pitidos por turnos,
 *			 respetando el estado (pitidos o sonido continuo) y el volumen de
 *			 cada uno.
 *			 Caso D: Ningun sensor activo, silencio total.
 * @note     USW.2.1.3.1
 **********************************************************************************/
void SC_ShowState(void)
{
    uint8_t  frontActive = 0;
    uint8_t  rearActive  = 0;
    static uint8_t  shift        = 0;
    static uint32_t shiftCounter = 0;
    static uint8_t  phaseCounter = 0;
    uint32_t shiftDuration;

    if((buzzer_front.mode != BUZZER_MODE_OFF)) frontActive = 1;
    if((buzzer_rear.mode != BUZZER_MODE_OFF)) rearActive = 1;

    buzzer_front.counter++;
    buzzer_rear.counter++;

    if( (frontActive==1) & (rearActive == 0) )
    {
        buzzer_rear.action = MUTE;
        Transistor_SetState(BUZZER_REAR, MUTE);

        if(buzzer_front.mode == BUZZER_MODE_CONTINUOUS)
        {
            Buzzer_ApplyVolume(buzzer_front.volume);
            buzzer_front.action = PLAY;
            Transistor_SetState(BUZZER_FRONT, PLAY);
        }
        else
        {
            if(buzzer_front.counter >= buzzer_front.threshold)
            {
                buzzer_front.counter     = 0;
                buzzer_front.action = !buzzer_front.action;

                if(buzzer_front.action) Buzzer_ApplyVolume(buzzer_front.volume);
                Transistor_SetState(BUZZER_FRONT, buzzer_front.action);
            }
        }
    }

    else if( (frontActive == 0) & (rearActive == 1) )
    {
        buzzer_front.action = MUTE;
        Transistor_SetState(BUZZER_FRONT, MUTE);

        if(buzzer_rear.mode == BUZZER_MODE_CONTINUOUS)
        {
            Buzzer_ApplyVolume(buzzer_rear.volume);
            buzzer_rear.action = PLAY;
            Transistor_SetState(BUZZER_REAR, PLAY);
        }
        else
        {
            if(buzzer_rear.counter >= buzzer_rear.threshold)
            {
                buzzer_rear.counter     = 0;
                buzzer_rear.action = !buzzer_rear.action;

                if(buzzer_rear.action) Buzzer_ApplyVolume(buzzer_rear.volume);
                Transistor_SetState(BUZZER_REAR, buzzer_rear.action);
            }
        }
    }

    else if( (frontActive == 1) & (rearActive == 1) )
    {
        if(shift == 0) shiftDuration = buzzer_front.threshold;
        else shiftDuration = buzzer_rear.threshold;

        shiftCounter++;

        if(shiftCounter >= shiftDuration)
        {
            shiftCounter = 0;
            phaseCounter++;

            if(shift == 0)
            {
                if(buzzer_front.mode == BUZZER_MODE_BEEP)
                {
                    buzzer_front.action = !buzzer_front.action;

                    if(buzzer_front.action) Buzzer_ApplyVolume(buzzer_front.volume);
                    Transistor_SetState(BUZZER_FRONT, buzzer_front.action);
                }

                if(phaseCounter >= 6)
                {
                    phaseCounter = 0;
                    shift = 1;

                    buzzer_front.action = MUTE;
                    Transistor_SetState(BUZZER_FRONT, MUTE);

                    buzzer_rear.action = PLAY;
                    Buzzer_ApplyVolume(buzzer_rear.volume);
                    Transistor_SetState(BUZZER_REAR, PLAY);
                }
            }
            else
            {
                if(buzzer_rear.mode == BUZZER_MODE_BEEP)
                {
                    buzzer_rear.action = !buzzer_rear.action;

                    if(buzzer_rear.action) Buzzer_ApplyVolume(buzzer_rear.volume);
                    Transistor_SetState(BUZZER_REAR, buzzer_rear.action);
                }

                if(phaseCounter >= 6)
                {
                    phaseCounter = 0;
                    shift = 0;

                    buzzer_rear.action = MUTE;
                    Transistor_SetState(BUZZER_REAR, PLAY);

                    buzzer_front.action = PLAY;
                    Buzzer_ApplyVolume(buzzer_front.volume);
                    Transistor_SetState(BUZZER_FRONT, PLAY);
                }
            }
        }
    }

    else
    {
        buzzer_front.action = MUTE;
        buzzer_rear.action  = MUTE;
        Transistor_SetState(BUZZER_FRONT, PLAY);
        Transistor_SetState(BUZZER_REAR, PLAY);
    }
}
