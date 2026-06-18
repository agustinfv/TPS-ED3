/*******************************************************************************//**
 * @file     leds.h
 * @brief    Driver de la interfaz de usuario óptica (LEDs RGB).
 * @details  Contiene las funciones para inicializar y controlar los colores de los
 *           LEDs RGB asociados a cada sensor.
 *           de pines GPIO.
 * @note     ESW.2.1.6 / 2.1.10
 **********************************************************************************/

#ifndef LEDS_H
#define LEDS_H

#include "proximity_assistant.h"
#include "gpio.h"


/*******************************************************************************//**
 * @brief    Macros con la correspondencia entre los pines y los LED RGB.
 **********************************************************************************/
#define LED_FRONT_PORT      2
#define LED_FRONT_R_PIN     0
#define LED_FRONT_G_PIN     1
#define LED_REAR_PORT       2
#define LED_REAR_R_PIN      2
#define LED_REAR_G_PIN      3

/*******************************************************************************//**
 * @brief    Enum con los colores disponibles para el LED RGB (no se usa azul).
 **********************************************************************************/
typedef enum
{
    LED_COLOR_OFF,
    LED_COLOR_GREEN,
    LED_COLOR_YELLOW,
    LED_COLOR_RED
} led_color;

/*******************************************************************************//**
 * @brief    Configura los pines de los LEDs RGB como salidas.
 * @details  Configura los pines correspondientes a los colores rojo y verde de
 *           ambos LEDs RGB y los inicializa en nivel bajo (apagados).
 * @note     USW.2.1.6.1 / 2.1.10.1
 **********************************************************************************/
void LED_Init(void);

/*******************************************************************************//**
 * @brief    Aplica un color específico al LED RGB de un sensor.
 * @details  Enciende y apaga los pines correspondientes para formar el color
 *           deseado (verde, rojo, amarillo o ninguno) en el LED RGB correspondiente.
 *           Hecho para LEDs RGB de ánod0 común
 * @note     USW.2.1.6.1 / 2.1.10.1
 *
 * @param id    SENSOR_FRONT o SENSOR_REAR.
 * @param color Color a aplicar (enum led_color).
 **********************************************************************************/
void LED_SetColor(sensor_id id, led_color color);

#endif
