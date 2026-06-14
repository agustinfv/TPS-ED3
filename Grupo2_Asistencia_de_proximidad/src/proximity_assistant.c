/*******************************************************************************//**
 * @file     proximity_assistant.c
 * @brief    Implementación del controlador principal del sistema.
 * @details  Procesa la situación del sistema según la distancia leída de los
 * 			 sensores HCSR04: actualiza los estados internos y maneja la respuesta
 * 			 en la interfaz de usuario (LEDs RGB y buzzers) de acuerdo a ellos,
 * @note     ESW.2.1.2
 **********************************************************************************/

#include "proximity_assistant.h"

/*******************************************************************************//**
 * @brief   Flags de nueva medición de distancia para cada sensor.
 **********************************************************************************/
volatile uint8_t distFrontFlag = 0;
volatile uint8_t distRearFlag  = 0;

/*******************************************************************************//**
 * @brief    Variables con los struct de cada sensor.
 **********************************************************************************/
sensor_t sensor_front;
sensor_t sensor_rear;

/*******************************************************************************//**
 * @brief    Inicializa los struct de ambos sensores con valores por defecto.
 * @details  Asigna id, state = SAFE_ZONE y distance = 100 cm a ambos sensores.
 * @note     USW.2.1.2.1
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
 * @brief    Según la distancia asigna el estado y levanta la flag de nueva medición.
 * @details  Obtiene la distancia del sensor, la compara con los umbrales  y
 *  		 actualiza el struct global del sensor. Finalmente pone en 1 la
 *  		 correspondiente flag de nueva medición del sensor.
 * @note     USW.2.1.2.1
 *
 * @param id      SENSOR_FRONT o SENSOR_REAR.
 * @param time_us Tiempo en alto del pulso ECHO en µs.
 **********************************************************************************/
void PA_SetState(sensor_id id, uint32_t time_us)
{
    uint32_t dist;

    dist = HCSR04_GetDistanceCm(time_us);

    if(id == SENSOR_FRONT)
    {
        sensor_front.distance = dist;

        if(dist >= DIST_WARNING_FAR) sensor_front.state = SAFE_ZONE;
        else if(dist >= DIST_WARNING_NEAR) sensor_front.state = WARNING_ZONE;
        else sensor_front.state = DANGEROUS_ZONE;

        distFrontFlag = 1;
    }
    else
    {
        sensor_rear.distance = dist;

        if(dist >= DIST_WARNING_FAR) sensor_rear.state = SAFE_ZONE;
        else if(dist >= DIST_WARNING_NEAR) sensor_rear.state = WARNING_ZONE;
        else sensor_rear.state = DANGEROUS_ZONE;

        distRearFlag = 1;
    }
}

/*******************************************************************************//**
 * @brief    Controla la interfaz de usuario correspondiente al sensor indicado.
 * @details  De acuerdo al estado en el que se encuentre el sistema según la
 * 			 distancia medida por el sensor, actualiza el correspondiente LED RGB y
 * 			 configura el modo de operación del correspondiente buzzer, así como
 * 			 su volumen.
 * @note     USW.2.1.2.1
 *
 * @param id SENSOR_FRONT o SENSOR_REAR.
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

    switch(state)
    {
    case SAFE_ZONE:
        LED_SetColor(id, LED_COLOR_GREEN);
        break;
    case WARNING_ZONE:
        LED_SetColor(id, LED_COLOR_YELLOW);
        break;
    case DANGEROUS_ZONE:
        LED_SetColor(id, LED_COLOR_RED);
        break;
    default:
        LED_SetColor(id, LED_COLOR_OFF);
        break;
    }

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
