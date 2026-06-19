/*******************************************************************************//**
 * @file     sound_ctrl.h
 * @brief    Servicio de control de sonido y señalización óptica.
 * @details  Provee la lógica de intercalado de buzzers, control de volumen,
 *           modos de pitido y control de los LEDs RGB. Oculta los detalles
 *           del hardware (transistores, DAC, DMA, GPIO) detrás de una interfaz
 *           orientada al comportamiento del sistema.
 *           Re-exporta buzzer_id, buzzer_action, led_color y sensor_id para
 *           que la capa de aplicación pueda operar sin conocer el driver.
 * @note     ESW - Capa de Servicios
 **********************************************************************************/

#ifndef SOUND_CTRL_H
#define SOUND_CTRL_H

#include <stdint.h>
#include "devices/alarma.h"

/*******************************************************************************//**
 * @brief    Macros de volumen según el estado del sistema.
 **********************************************************************************/
#define VOL_SAFE            0
#define VOL_WARNING_LOW     25
#define VOL_WARNING_HIGH    50
#define VOL_DANGER_LOW      75
#define VOL_DANGER_HIGH     100

/*******************************************************************************//**
 * @brief    Semiperíodos en ms para cada frecuencia de pitido.
 * @details  El threshold define cuántos ms dura cada mitad del ciclo (sonido +
 *           silencio). El período completo es el doble. Con base de tiempo de 1 ms:
 *           THRESHOLD_WARNING_LOW  → pitido a 2 Hz.
 *           THRESHOLD_WARNING_HIGH → pitido a 4 Hz.
 *           THRESHOLD_DANGER_LOW   → pitido a ~8 Hz.
 **********************************************************************************/
#define THRESHOLD_WARNING_LOW   250
#define THRESHOLD_WARNING_HIGH  125
#define THRESHOLD_DANGER_LOW     62

/*******************************************************************************//**
 * @brief    Modos de operación del buzzer.
 **********************************************************************************/
typedef enum
{
    BUZZER_MODE_OFF,
    BUZZER_MODE_BEEP,
    BUZZER_MODE_CONTINUOUS
} buzzer_mode;

/*******************************************************************************//**
 * @brief    Estado lógico completo de un buzzer.
 **********************************************************************************/
typedef struct
{
    buzzer_id     id;
    buzzer_mode   mode;
    buzzer_action action;
    uint32_t      threshold;
    uint32_t      counter;
    uint8_t       volume;
} buzzer_t;

/*******************************************************************************//**
 * @brief    Estados lógicos de ambos buzzers, accesibles desde otros módulos.
 **********************************************************************************/
extern buzzer_t buzzer_front;
extern buzzer_t buzzer_rear;

/*******************************************************************************//**
 * @brief    Inicializa el servicio de sonido y el hardware de la alarma.
 * @details  Inicializa el hardware (Timer1, DAC, DMA, LEDs, transistores) y
 *           carga los valores por defecto de los structs de los buzzers:
 *           apagados, pitido a 2 Hz, volumen 0.
 **********************************************************************************/
void SC_Init(void);

/*******************************************************************************//**
 * @brief    Establece el modo de operación del buzzer indicado.
 *
 * @param id    BUZZER_FRONT o BUZZER_REAR.
 * @param mode  BUZZER_MODE_OFF, BUZZER_MODE_BEEP o BUZZER_MODE_CONTINUOUS.
 **********************************************************************************/
void SC_SetMode(buzzer_id id, buzzer_mode mode);

/*******************************************************************************//**
 * @brief    Asigna el volumen al buzzer indicado.
 *
 * @param id      BUZZER_FRONT o BUZZER_REAR.
 * @param volume  Nivel de volumen de 0 a 100.
 **********************************************************************************/
void SC_SetVolume(buzzer_id id, uint8_t volume);

/*******************************************************************************//**
 * @brief    Actualiza el semiperíodo de pitido del buzzer indicado.
 *
 * @param id         BUZZER_FRONT o BUZZER_REAR.
 * @param threshold  Semiperíodo en ms. Debe ser mayor a 0.
 **********************************************************************************/
void SC_SetThreshold(buzzer_id id, uint32_t threshold);

/*******************************************************************************//**
 * @brief    Aplica el estado lógico de ambos buzzers al hardware.
 * @details  Evalúa qué buzzers están activos y aplica la lógica correspondiente:
 *           Caso A: solo frontal activo → opera según su modo.
 *           Caso B: solo trasero activo → opera según su modo.
 *           Caso C: ambos activos → intercalado de 3 pitidos por turno.
 *           Caso D: ninguno activo → silencio total.
 *           Debe llamarse cada 1 ms desde la ISR del Timer1.
 **********************************************************************************/
void SC_ShowState(void);

/*******************************************************************************//**
 * @brief    Aplica un color al LED RGB del sensor indicado.
 * @details  Delega en el driver de alarma. Permite a la capa de aplicación
 *           controlar los LEDs sin conocer el hardware.
 *
 * @param id    SENSOR_FRONT o SENSOR_REAR.
 * @param color Color a aplicar (led_color).
 **********************************************************************************/
void SC_SetLED(sensor_id id, led_color color);

#endif
