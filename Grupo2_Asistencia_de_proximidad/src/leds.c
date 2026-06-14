/*******************************************************************************//**
 * @file     leds.c
 * @brief    Implementación del driver de los LEDs RGB.
 * @details  Se encarga de configurar el GPIO para los pines de los LEDs y aplicar
 * 		     los estados lógicos para lograr los colores requeridos por el sistema.
 * @note     ESW.2.1.6 / 2.1.10
 **********************************************************************************/

#include "leds.h"

/*******************************************************************************//**
 * @brief    Configura los pines de los LEDs RGB como salidas.
 * @details  Configura los pines P2.0, P2.1 (Frontal) y P2.2, P2.3 (Trasero) como
 *           salidas GPIO y los inicializa en nivel bajo (apagados).
 * @note     USW.2.1.6.1 / 2.1.10.1
 **********************************************************************************/
void LED_Init(void)
{
    GPIO_ConfigPin(LED_FRONT_PORT, LED_FRONT_R_PIN, 1);
    GPIO_ConfigPin(LED_FRONT_PORT, LED_FRONT_G_PIN, 1);
    GPIO_ConfigPin(LED_REAR_PORT,  LED_REAR_R_PIN,  1);
    GPIO_ConfigPin(LED_REAR_PORT,  LED_REAR_G_PIN,  1);

    GPIO_ClearValue(LED_FRONT_PORT, (1 << LED_FRONT_R_PIN) | (1 << LED_FRONT_G_PIN));
    GPIO_ClearValue(LED_REAR_PORT, (1 << LED_REAR_R_PIN)  | (1 << LED_REAR_G_PIN));
}

/*******************************************************************************//**
 * @brief    Aplica un color específico al LED RGB de un sensor.
 * @details  Controla los pines rojo y verde del sensor indicado para formar el
 *           color pedido en el LED RGB, apagando ambos para LED_COLOR_OFF,
 *           activando el verde para LED_COLOR_GREEN, etc. Hecho para LEDs RGB de
 *           ánod0 común
 * @note     USW.2.1.6.1 / 2.1.10.1
 *
 * @param id    SENSOR_FRONT o SENSOR_REAR.
 * @param color Color a aplicar (enum led_color).
 **********************************************************************************/
void LED_SetColor(sensor_id id, led_color color)
{
    uint8_t port;
    uint8_t rPin;
    uint8_t gPin;

    if(id == SENSOR_FRONT)
    {
        port = LED_FRONT_PORT;
        rPin = LED_FRONT_R_PIN;
        gPin = LED_FRONT_G_PIN;
    }
    else
    {
        port = LED_REAR_PORT;
        rPin = LED_REAR_R_PIN;
        gPin = LED_REAR_G_PIN;
    }

    switch(color)
    {
    case LED_COLOR_OFF:
        GPIO_SetValue(port, (1 << rPin));
        GPIO_SetValue(port, (1 << gPin));
        break;

    case LED_COLOR_GREEN:
        GPIO_SetValue(port, (1 << rPin));
        GPIO_ClearValue(port, (1 << gPin));
        break;

    case LED_COLOR_YELLOW:
        GPIO_ClearValue(port, (1 << rPin) | (1 << gPin));
        break;

    case LED_COLOR_RED:
        GPIO_ClearValue(port, (1 << rPin));
        GPIO_SetValue(port, (1 << gPin));
        break;
    default:
    	break;
    }
}
