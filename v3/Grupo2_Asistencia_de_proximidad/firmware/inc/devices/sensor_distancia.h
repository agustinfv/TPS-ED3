/*******************************************************************************//**
 * @file     sensor_distancia.h
 * @brief    Driver del sensor ultrasónico HC-SR04.
 * @details  Encapsula toda la configuración de hardware necesaria para operar
 *           el sensor: pines TRIG como GPIO salida (P0.0 y P0.1), pines ECHO
 *           como función alternativa CAP0.0 y CAP0.1 (P1.26 y P1.27), y el
 *           Timer0 en modo capture con interrupción en ambos flancos.
 *           Los pines ECHO y el Timer0 se exponen mínimamente para que la ISR
 *           del Timer0 en main.c pueda leer los registros de capture y el estado
 *           del pin, sin conocer el resto de la implementación.
 * @note     ESW - Capa de Dispositivos
 **********************************************************************************/

#ifndef SENSOR_DISTANCIA_H
#define SENSOR_DISTANCIA_H

#include <stdint.h>
#include "LPC17xx.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"

/*******************************************************************************//**
 * @brief    Identificadores de los dos sensores del sistema.
 **********************************************************************************/
typedef enum
{
    SENSOR_FRONT,
    SENSOR_REAR
} sensor_id;

/*******************************************************************************//**
 * @brief    Pines ECHO expuestos para lectura de flanco en la ISR del Timer0.
 * @details  La ISR necesita leer GPIO_ReadValue() sobre estos puertos y máscaras
 *           para distinguir flanco de subida (inicio ECHO) de bajada (fin ECHO).
 **********************************************************************************/
#define ECHO_FRONT_PORT     1
#define ECHO_FRONT_MASK     (1 << 26)
#define ECHO_REAR_PORT      1
#define ECHO_REAR_MASK      (1 << 27)

/*******************************************************************************//**
 * @brief    Inicializa los pines TRIG/ECHO y el Timer0 en modo capture.
 * @details  Configura TRIG como GPIO salida (bajo), ECHO como CAP0.0 y CAP0.1,
 *           Timer0 con prescaler de 1 µs, capture en ambos flancos con
 *           interrupción, y habilita el contador y el NVIC (prioridad 1).
 **********************************************************************************/
void SensorDist_Init(void);

/*******************************************************************************//**
 * @brief    Convierte el tiempo del pulso ECHO a distancia en centímetros.
 * @details  Fórmula: d (cm) = (time_us * 17) / 1000, usando aritmética entera.
 *
 * @param time_us  Duración del pulso ECHO en microsegundos.
 * @return         Distancia en centímetros.
 **********************************************************************************/
uint32_t SensorDist_GetDistanceCm(uint32_t time_us);

/*******************************************************************************//**
 * @brief    Genera los pulsos TRIG de ambos sensores.
 * @details  Debe llamarse cada 1 ms desde la ISR del Timer1. Mantiene bajo TRIG
 *           durante TRIG_PERIOD_MS ms y luego lo pone en alto 1 ms para disparar
 *           la medición.
 **********************************************************************************/
void SensorDist_TriggerUpdate(void);

#endif
