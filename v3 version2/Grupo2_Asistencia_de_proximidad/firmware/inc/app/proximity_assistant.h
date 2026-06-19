/*******************************************************************************//**
 * @file     proximity_assistant.h
 * @brief    Controlador principal del sistema de asistencia de proximidad.
 * @details  Procesa las distancias medidas por los sensores, determina el estado
 *           del sistema (seguro, advertencia, peligro) y actualiza la interfaz
 *           de usuario óptica y acústica a través de los servicios de distancia
 *           y control de sonido.
 *           No conoce la implementación del hardware: toda interacción con
 *           dispositivos se realiza a través de la capa de servicios.
 * @note     ESW - Capa de Aplicación
 **********************************************************************************/

#ifndef PROXIMITY_ASSISTANT_H
#define PROXIMITY_ASSISTANT_H

#include "services/distancia.h"
#include "services/alarm_control.h"

/*******************************************************************************//**
 * @brief    Umbrales de distancia en centímetros para cada estado del sistema.
 **********************************************************************************/
#define DIST_WARNING_FAR    80
#define DIST_WARNING_MID    60
#define DIST_WARNING_NEAR   40
#define DIST_DANGER_NEAR    20

/*******************************************************************************//**
 * @brief    Estados posibles del sistema según la distancia medida.
 **********************************************************************************/
typedef enum
{
    SAFE_ZONE,
    WARNING_ZONE,
    DANGEROUS_ZONE
} sensor_state;

/*******************************************************************************//**
 * @brief    Representa el estado actual de un sensor.
 **********************************************************************************/
typedef struct
{
    sensor_id    id;
    sensor_state state;
    uint32_t     distance;
} sensor_t;

/*******************************************************************************//**
 * @brief    Estados actuales de ambos sensores, accesibles desde main.c.
 **********************************************************************************/
extern sensor_t         sensor_front;
extern sensor_t         sensor_rear;

/*******************************************************************************//**
 * @brief    Flags que indican nueva medición disponible para cada sensor.
 * @details  Se ponen en 1 desde la ISR del Timer0 (vía PA_SetState) y se
 *           consumen en el bucle principal de main.c.
 **********************************************************************************/
extern volatile uint8_t distFrontFlag;
extern volatile uint8_t distRearFlag;

/*******************************************************************************//**
 * @brief    Inicializa los estados de ambos sensores con valores por defecto.
 * @details  Asigna id, state = SAFE_ZONE y distance = 100 cm a cada sensor.
 **********************************************************************************/
void PA_Init(void);

/*******************************************************************************//**
 * @brief    Calcula la distancia y actualiza el estado del sensor indicado.
 * @details  Convierte el tiempo del ECHO a cm mediante DIST_GetDistanceCm(),
 *           compara con los umbrales para asignar el estado, y levanta la
 *           flag de nueva medición correspondiente. Se llama desde la ISR
 *           del Timer0.
 *
 * @param id      SENSOR_FRONT o SENSOR_REAR.
 * @param time_us Duración del pulso ECHO en microsegundos.
 **********************************************************************************/
void PA_SetState(sensor_id id, uint32_t time_us);

/*******************************************************************************//**
 * @brief    Actualiza la interfaz de usuario para el sensor indicado.
 * @details  Según el estado del sensor, configura el LED RGB (a través del
 *           servicio AC_SetLED) y el buzzer (modo, volumen, threshold a través
 *           de AC_SetMode, AC_SetVolume y AC_SetThreshold).
 *           Se llama desde el bucle principal cuando hay nueva medición.
 *
 * @param id  SENSOR_FRONT o SENSOR_REAR.
 **********************************************************************************/
void PA_ManageState(sensor_id id);

#endif
