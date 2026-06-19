/*******************************************************************************//**
 * @file     distancia.c
 * @brief    Implementación del servicio de medición de distancia.
 * @details  Actúa como interfaz entre la capa de aplicación y el driver
 *           sensor_distancia. No contiene lógica de hardware: toda la
 *           configuración de GPIO y Timer0 ocurre dentro del driver de dispositivo.
 * @note     ESW - Capa de Servicios
 **********************************************************************************/

#include "services/distancia.h"

/*******************************************************************************//**
 * @brief    Inicializa el hardware del sensor delegando en el driver de dispositivo.
 **********************************************************************************/
void DIST_Init(void)
{
    SensorDist_Init();
}

/*******************************************************************************//**
 * @brief    Convierte tiempo de ECHO a distancia delegando en el driver.
 **********************************************************************************/
uint32_t DIST_GetDistanceCm(uint32_t time_us)
{
    return SensorDist_GetDistanceCm(time_us);
}

/*******************************************************************************//**
 * @brief    Genera los pulsos TRIG delegando en el driver de dispositivo.
 **********************************************************************************/
void DIST_TriggerUpdate(void)
{
    SensorDist_TriggerUpdate();
}
