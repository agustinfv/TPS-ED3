/*******************************************************************************//**
 * @file     proximity_assistant.c
 * @brief    Implementación del controlador principal del sistema.
 * @details  Procesa las distancias medidas por los sensores HC-SR04 a través del
 *           servicio DIST_GetDistanceCm(), actualiza los estados internos y
 *           controla la interfaz de usuario (LEDs RGB y buzzers) mediante los
 *           servicios SC_SetLED(), SC_SetMode(), SC_SetVolume() y SC_SetThreshold().
 *           No conoce la implementación del hardware: toda interacción con
 *           dispositivos ocurre a través de la capa de servicios.
 * @note     ESW - Capa de Aplicación
 **********************************************************************************/

#include "app/proximity_assistant.h"

/*******************************************************************************//**
 * @brief    Flags de nueva medición de distancia para cada sensor.
 **********************************************************************************/
volatile uint8_t distFrontFlag = 0;
volatile uint8_t distRearFlag  = 0;

/*******************************************************************************//**
 * @brief    Estado actual de cada sensor.
 **********************************************************************************/
sensor_t sensor_front;
sensor_t sensor_rear;

/*******************************************************************************//**
 * @brief    Inicializa los estados de ambos sensores con valores por defecto.
 **********************************************************************************/
void PA_Init(void)
{
    sensor_front.id       = SENSOR_FRONT;
    sensor_front.state    = SAFE_ZONE;
    sensor_front.distance = 100;

    sensor_rear.id        = SENSOR_REAR;
    sensor_rear.state     = SAFE_ZONE;
    sensor_rear.distance  = 100;
}

/*******************************************************************************//**
 * @brief    Calcula la distancia y actualiza el estado del sensor indicado.
 * @details  Usa DIST_GetDistanceCm() para convertir el tiempo del ECHO a cm,
 *           compara con los umbrales para asignar el estado lógico del sensor
 *           y levanta la flag de nueva medición.
 *           Llamada desde la ISR del Timer0 en main.c.
 **********************************************************************************/
void PA_SetState(sensor_id id, uint32_t time_us)
{
    uint32_t dist = DIST_GetDistanceCm(time_us);

    if(id == SENSOR_FRONT)
    {
        sensor_front.distance = dist;

        if(dist >= DIST_WARNING_FAR)        sensor_front.state = SAFE_ZONE;
        else if(dist >= DIST_WARNING_NEAR)  sensor_front.state = WARNING_ZONE;
        else                                sensor_front.state = DANGEROUS_ZONE;

        distFrontFlag = 1;
    }
    else
    {
        sensor_rear.distance = dist;

        if(dist >= DIST_WARNING_FAR)        sensor_rear.state = SAFE_ZONE;
        else if(dist >= DIST_WARNING_NEAR)  sensor_rear.state = WARNING_ZONE;
        else                                sensor_rear.state = DANGEROUS_ZONE;

        distRearFlag = 1;
    }
}

/*******************************************************************************//**
 * @brief    Actualiza la interfaz de usuario para el sensor indicado.
 * @details  Según el estado y la distancia, configura el LED RGB mediante
 *           SC_SetLED() y el buzzer mediante SC_SetMode(), SC_SetVolume() y
 *           SC_SetThreshold(). Llamada desde el bucle principal de main.c.
 **********************************************************************************/
void PA_ManageState(sensor_id id)
{
    uint32_t     dist;
    sensor_state state;
    buzzer_id    buzzer;

    if(id == SENSOR_FRONT)
    {
        dist   = sensor_front.distance;
        state  = sensor_front.state;
        buzzer = BUZZER_FRONT;
    }
    else
    {
        dist   = sensor_rear.distance;
        state  = sensor_rear.state;
        buzzer = BUZZER_REAR;
    }

    /* ---- Control del LED RGB ---- */
    switch(state)
    {
    case SAFE_ZONE:      SC_SetLED(id, LED_COLOR_GREEN);  break;
    case WARNING_ZONE:   SC_SetLED(id, LED_COLOR_YELLOW); break;
    case DANGEROUS_ZONE: SC_SetLED(id, LED_COLOR_RED);    break;
    default:             SC_SetLED(id, LED_COLOR_OFF);    break;
    }

    /* ---- Control del buzzer ---- */
    switch(state)
    {
    case SAFE_ZONE:
        SC_SetMode(buzzer, BUZZER_MODE_OFF);
        SC_SetVolume(buzzer, VOL_SAFE);
        break;

    case WARNING_ZONE:
        SC_SetMode(buzzer, BUZZER_MODE_BEEP);

        if(dist >= DIST_WARNING_MID)
        {
            SC_SetThreshold(buzzer, THRESHOLD_WARNING_LOW);
            SC_SetVolume(buzzer, VOL_WARNING_LOW);
        }
        else
        {
            SC_SetThreshold(buzzer, THRESHOLD_WARNING_HIGH);
            SC_SetVolume(buzzer, VOL_WARNING_HIGH);
        }
        break;

    case DANGEROUS_ZONE:
        if(dist >= DIST_DANGER_NEAR)
        {
            SC_SetMode(buzzer, BUZZER_MODE_BEEP);
            SC_SetThreshold(buzzer, THRESHOLD_DANGER_LOW);
            SC_SetVolume(buzzer, VOL_DANGER_LOW);
        }
        else
        {
            SC_SetMode(buzzer, BUZZER_MODE_CONTINUOUS);
            SC_SetVolume(buzzer, VOL_DANGER_HIGH);
        }
        break;

    default:
        break;
    }
}
