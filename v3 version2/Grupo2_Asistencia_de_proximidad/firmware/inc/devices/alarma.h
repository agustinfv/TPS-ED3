/*******************************************************************************//**
 * @file     alarma.h
 * @brief    Driver de la alarma: LEDs RGB, transistores, DAC y DMA.
 * @details  Encapsula toda la configuración de hardware de la interfaz de salida:
 *           GPIO de los LEDs RGB (P2.0-P2.3) y de los transistores NPN (P0.4 y P0.5),
 *           Timer1 como base de tiempo de 1 ms, DAC configurado para 440 Hz via DMA,
 *           y el canal 0 del GPDMA con LLI circular para reproducción continua de
 *           la onda senoidal.
 *           Expone funciones de control sin revelar pines, registros ni constantes
 *           del hardware.
 * @note     ESW - Capa de Dispositivos
 **********************************************************************************/

#ifndef ALARMA_H
#define ALARMA_H

#include <stdint.h>
#include "devices/sensor_distancia.h"

/*******************************************************************************//**
 * @brief    Colores disponibles para los LEDs RGB del sistema.
 **********************************************************************************/
typedef enum
{
    LED_COLOR_OFF,
    LED_COLOR_GREEN,
    LED_COLOR_YELLOW,
    LED_COLOR_RED
} led_color;

/*******************************************************************************//**
 * @brief    Identificadores de los dos buzzers del sistema.
 **********************************************************************************/
typedef enum
{
    BUZZER_FRONT,
    BUZZER_REAR
} buzzer_id;

/*******************************************************************************//**
 * @brief    Estado de encendido o silencio de un buzzer.
 * @details  MUTE = transistor saturado = buzzer en silencio.
 *           PLAY = transistor en corte  = buzzer suena.
 **********************************************************************************/
typedef enum
{
    MUTE = 0,
    PLAY = 1
} buzzer_action;

/*******************************************************************************//**
 * @brief    Inicializa todo el hardware de la alarma.
 * @details  Configura en este orden:
 *           1. GPIO de LEDs como salidas (bajo = apagados).
 *           2. GPIO de transistores como salidas (alto = buzzers en silencio).
 *           3. Timer1 con match de 1 ms, interrupción habilitada (prioridad 2).
 *           4. DAC en P0.26 (AOUT) con timeout para 440 Hz.
 *           5. GPDMA canal 0 con LLI circular hacia el registro DACR.
 *           6. Carga la onda senoidal con volumen 0.
 **********************************************************************************/
void Alarma_Init(void);

/*******************************************************************************//**
 * @brief    Aplica un color al LED RGB del sensor indicado.
 * @details  Controla los pines rojo y verde del LED correspondiente al sensor
 *           para obtener el color pedido. LEDs de cátodo común.
 *
 * @param id    SENSOR_FRONT o SENSOR_REAR.
 * @param color Color a aplicar (led_color).
 **********************************************************************************/
void Alarma_SetLED(sensor_id id, led_color color);

/*******************************************************************************//**
 * @brief    Aplica un estado de encendido o silencio al transistor del buzzer.
 * @details  PLAY pone el pin en bajo (transistor en corte, buzzer suena).
 *           MUTE pone el pin en alto (transistor saturado, silencio).
 *
 * @param id      BUZZER_FRONT o BUZZER_REAR.
 * @param action  PLAY o MUTE.
 **********************************************************************************/
void Alarma_SetTransistor(buzzer_id id, buzzer_action action);

/*******************************************************************************//**
 * @brief    Ajusta la amplitud de la onda senoidal según el volumen indicado.
 * @details  Recalcula las 32 muestras de la LUT escalando la amplitud, detiene
 *           brevemente el DMA para evitar mezcla de muestras, y lo recarga desde
 *           la LLI para reiniciar desde la muestra 0.
 *
 * @param volume  Nivel de volumen: 0 (silencio) a 100 (máximo).
 **********************************************************************************/
void Alarma_ApplyVolume(uint8_t volume);

#endif
