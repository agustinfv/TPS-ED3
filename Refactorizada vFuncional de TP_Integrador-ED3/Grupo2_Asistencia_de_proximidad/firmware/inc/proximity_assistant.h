/*******************************************************************************//**
 * @file     proximity_assistant.h
 * @brief    Controlador principal del sistema de asistencia de proximidad.
 * @details  Se encarga de procesar las distancias medidas por los sensores HC-SR04,
 *           determinar los estados según la misma (seguro, advertencia, peligro) y
 *           manejar tanto la señalización óptica (LEDs RGB) como la acústica
 *           (buzzer). Almacena el estado actual de los sensores.
 * @note     ESW.2.1.2
 **********************************************************************************/

#ifndef PROXIMITY_ASSISTANT_H
#define PROXIMITY_ASSISTANT_H

#include "hcsr04.h"
#include "sound_ctrl.h"
#include "leds.h"

/*******************************************************************************//**
 * @brief    Macros con los umbrales de distancia dentro de cada estado.
 **********************************************************************************/
#define DIST_WARNING_FAR    80
#define DIST_WARNING_MID    60
#define DIST_WARNING_NEAR    40
#define DIST_DANGER_NEAR    20



/*******************************************************************************//**
 * @brief    Enum con los estados de cada sensor según la distancia medida.
 **********************************************************************************/
typedef enum
{
    SAFE_ZONE,
    WARNING_ZONE,
    DANGEROUS_ZONE
} sensor_state;

/*******************************************************************************//**
 * @brief    Struct con los datos relevantes los sensores.
 * @details  Agrupa la identificación, el estado según la distancia y la última
 *           distancia medida en centímetros.
 **********************************************************************************/
typedef struct
{
    sensor_id    id;
    sensor_state state;
    uint32_t     distance;
} sensor_t;

/*******************************************************************************//**
 * @brief    Variables con los struct de cada sensor, accesibles para otros módulos.
 **********************************************************************************/
extern sensor_t sensor_front;
extern sensor_t sensor_rear;

/*******************************************************************************//**
 * @brief    Flags de nueva medición de distancia de cada sensor, accesibles para
 * 			 otros módulos
 **********************************************************************************/
extern volatile uint8_t distFrontFlag;
extern volatile uint8_t distRearFlag;

/*******************************************************************************//**
 * @brief    Inicializa la máquina de estados de los sensores.
 * @details  Establece el estado inicial de sensor_front y sensor_rear:
 *           id, state = SAFE_ZONE, distance = 100 cm.
 * @note     USW.2.1.2.1
 **********************************************************************************/
void PA_Init(void);

/*******************************************************************************//**
 * @brief    Establece el estado de un sensor a partir del tiempo del ECHO.
 * @details  Toma la distancia a partir del tiempo del pulso ECHO, actualiza el
 *           estado del sensor indicado en su struct y pone en 1 la flag de
 *           nueva medición.
 *           Según la distancia, los estados se dan de la siguiente manera:
 *           dist >= 80 cm -> SAFE_ZONE.
 *           80 cm < dist >= 40 cm -> WARNING_ZONE.
 *           dist <  40 cm -> DANGEROUS_ZONE.
 * @note     USW.2.1.2.1
 *
 * @param id      SENSOR_FRONT o SENSOR_REAR.
 * @param time_us Duración del pulso ECHO en µs.
 **********************************************************************************/
void PA_SetState(sensor_id id, uint32_t time_us);

/*******************************************************************************//**
 * @brief    Controla la interfaz de usuario correspondiente al sensor indicado.
 * @details  Actualiza el LED RGB y configura el buzzer correspondiente (modo,
 *           volumen, threshold) según la distancia medida.
 *           Los umbrales dentro de cada estado, según la distancia, son:
 *             SAFE_ZONE:
 *             dist >= DIST_WARNING_FAR -> sin advertencia -> VOL_SAFE.
 *             WARNING_ZONE:
 *             dist >= DIST_WARNING_MID -> advertencia baja -> VOL_WARNING_LOW.
 *             dist <  DIST_WARNING_MID -> advertencia alta -> VOL_WARNING_HIGH.
 *             DANGEROUS_ZONE:
 *             dist >= DIST_DANGER_NEAR -> peligro bajo -> VOL_DANGER_LOW.
 *             dist <  DIST_DANGER_NEAR -> peligro alto -> VOL_DANGER_HIGH.
 * @note     USW.2.1.2.1
 *
 * @param id SENSOR_FRONT o SENSOR_REAR.
 **********************************************************************************/
void PA_ManageState(sensor_id id);

#endif
