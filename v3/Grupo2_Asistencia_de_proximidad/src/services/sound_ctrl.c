/*******************************************************************************//**
 * @file     sound_ctrl.c
 * @brief    Implementación del servicio de control de sonido y señalización.
 * @details  Contiene la lógica de intercalado de buzzers, control de volumen y
 *           modos de pitido. Delega toda interacción con el hardware en el
 *           driver de la alarma (Alarma_SetTransistor, Alarma_ApplyVolume,
 *           Alarma_SetLED), sin conocer pines, registros ni periféricos.
 * @note     ESW - Capa de Servicios
 **********************************************************************************/

#include "services/sound_ctrl.h"

/*******************************************************************************//**
 * @brief    Estados lógicos de ambos buzzers.
 **********************************************************************************/
buzzer_t buzzer_front;
buzzer_t buzzer_rear;

/*******************************************************************************//**
 * @brief    Inicializa el hardware de la alarma y los estados por defecto.
 **********************************************************************************/
void SC_Init(void)
{
    Alarma_Init();

    buzzer_front.id        = BUZZER_FRONT;
    buzzer_front.mode      = BUZZER_MODE_OFF;
    buzzer_front.action    = MUTE;
    buzzer_front.threshold = 250;
    buzzer_front.counter   = 0;
    buzzer_front.volume    = 0;

    buzzer_rear.id         = BUZZER_REAR;
    buzzer_rear.mode       = BUZZER_MODE_OFF;
    buzzer_rear.action     = MUTE;
    buzzer_rear.threshold  = 250;
    buzzer_rear.counter    = 0;
    buzzer_rear.volume     = 0;
}

/*******************************************************************************//**
 * @brief    Establece el modo de operación del buzzer indicado.
 **********************************************************************************/
void SC_SetMode(buzzer_id id, buzzer_mode mode)
{
    if(id == BUZZER_FRONT) buzzer_front.mode = mode;
    else                   buzzer_rear.mode  = mode;
}

/*******************************************************************************//**
 * @brief    Asigna el volumen al buzzer indicado (0–100).
 **********************************************************************************/
void SC_SetVolume(buzzer_id id, uint8_t volume)
{
    if(volume > 100) volume = 100;

    if(id == BUZZER_FRONT) buzzer_front.volume = volume;
    else                   buzzer_rear.volume  = volume;
}

/*******************************************************************************//**
 * @brief    Actualiza el semiperíodo de pitido del buzzer indicado.
 **********************************************************************************/
void SC_SetThreshold(buzzer_id id, uint32_t threshold)
{
    if(threshold == 0) return;

    if(id == BUZZER_FRONT) buzzer_front.threshold = threshold;
    else                   buzzer_rear.threshold  = threshold;
}

/*******************************************************************************//**
 * @brief    Aplica un color al LED del sensor indicado delegando en el driver.
 **********************************************************************************/
void SC_SetLED(sensor_id id, led_color color)
{
    Alarma_SetLED(id, color);
}

/*******************************************************************************//**
 * @brief    Aplica el estado lógico de los buzzers al hardware cada 1 ms.
 * @details  Evalúa qué buzzers están activos y opera en consecuencia:
 *           Caso A: solo frontal activo → opera según su modo independientemente.
 *           Caso B: solo trasero activo → opera según su modo independientemente.
 *           Caso C: ambos activos → intercalado de 3 pitidos por turno, con el
 *                   volumen y modo de cada buzzer respetados.
 *           Caso D: ninguno activo → silencio total en ambos canales.
 **********************************************************************************/
void SC_ShowState(void)
{
    uint8_t  frontActive  = (buzzer_front.mode != BUZZER_MODE_OFF) ? 1 : 0;
    uint8_t  rearActive   = (buzzer_rear.mode  != BUZZER_MODE_OFF) ? 1 : 0;

    static uint8_t  shift        = 0;
    static uint32_t shiftCounter = 0;
    static uint8_t  phaseCounter = 0;

    uint32_t shiftDuration;

    buzzer_front.counter++;
    buzzer_rear.counter++;

    /* ---- Caso A: solo frontal activo ---- */
    if(frontActive && !rearActive)
    {
        buzzer_rear.action = MUTE;
        Alarma_SetTransistor(BUZZER_REAR, MUTE);

        if(buzzer_front.mode == BUZZER_MODE_CONTINUOUS)
        {
            Alarma_ApplyVolume(buzzer_front.volume);
            buzzer_front.action = PLAY;
            Alarma_SetTransistor(BUZZER_FRONT, PLAY);
        }
        else
        {
            if(buzzer_front.counter >= buzzer_front.threshold)
            {
                buzzer_front.counter = 0;
                buzzer_front.action  = !buzzer_front.action;

                if(buzzer_front.action) Alarma_ApplyVolume(buzzer_front.volume);
                Alarma_SetTransistor(BUZZER_FRONT, buzzer_front.action);
            }
        }
    }

    /* ---- Caso B: solo trasero activo ---- */
    else if(!frontActive && rearActive)
    {
        buzzer_front.action = MUTE;
        Alarma_SetTransistor(BUZZER_FRONT, MUTE);

        if(buzzer_rear.mode == BUZZER_MODE_CONTINUOUS)
        {
            Alarma_ApplyVolume(buzzer_rear.volume);
            buzzer_rear.action = PLAY;
            Alarma_SetTransistor(BUZZER_REAR, PLAY);
        }
        else
        {
            if(buzzer_rear.counter >= buzzer_rear.threshold)
            {
                buzzer_rear.counter = 0;
                buzzer_rear.action  = !buzzer_rear.action;

                if(buzzer_rear.action) Alarma_ApplyVolume(buzzer_rear.volume);
                Alarma_SetTransistor(BUZZER_REAR, buzzer_rear.action);
            }
        }
    }

    /* ---- Caso C: ambos activos → intercalado de 3 pitidos por turno ---- */
    else if(frontActive && rearActive)
    {
        shiftDuration = (shift == 0) ? buzzer_front.threshold : buzzer_rear.threshold;

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

                    if(buzzer_front.action) Alarma_ApplyVolume(buzzer_front.volume);
                    Alarma_SetTransistor(BUZZER_FRONT, buzzer_front.action);
                }

                if(phaseCounter >= 6)
                {
                    phaseCounter = 0;
                    shift        = 1;

                    buzzer_front.action = MUTE;
                    Alarma_SetTransistor(BUZZER_FRONT, MUTE);

                    buzzer_rear.action = PLAY;
                    Alarma_ApplyVolume(buzzer_rear.volume);
                    Alarma_SetTransistor(BUZZER_REAR, PLAY);
                }
            }
            else
            {
                if(buzzer_rear.mode == BUZZER_MODE_BEEP)
                {
                    buzzer_rear.action = !buzzer_rear.action;

                    if(buzzer_rear.action) Alarma_ApplyVolume(buzzer_rear.volume);
                    Alarma_SetTransistor(BUZZER_REAR, buzzer_rear.action);
                }

                if(phaseCounter >= 6)
                {
                    phaseCounter = 0;
                    shift        = 0;

                    buzzer_rear.action = MUTE;
                    Alarma_SetTransistor(BUZZER_REAR, MUTE);

                    buzzer_front.action = PLAY;
                    Alarma_ApplyVolume(buzzer_front.volume);
                    Alarma_SetTransistor(BUZZER_FRONT, PLAY);
                }
            }
        }
    }

    /* ---- Caso D: ninguno activo → silencio total ---- */
    else
    {
        buzzer_front.action = MUTE;
        buzzer_rear.action  = MUTE;
        Alarma_ApplyVolume(0);
        Alarma_SetTransistor(BUZZER_FRONT, MUTE);
        Alarma_SetTransistor(BUZZER_REAR,  MUTE);
    }
}
