/*******************************************************************************//**
 * @file     hcsr04.h
 * @brief    Driver del sensor ultrasónico HC-SR04 usando capture del Timer0.
 * @details  Configura los pines TRIG (P0.0 y P0.1) y ECHO (CAP0.0 y CAP0.1), Timer0
 * 			 para capturar el pulso ECHO, calcula la distancia y envía el pulso
 * 			 TRIG para nueva medición de distancia.
 *
 * @note     ESW.2.1.7 / 2.1.20
 **********************************************************************************/

#ifndef HCSR04_H
#define HCSR04_H

#include <stdint.h>
#include "timer.h"
#include "gpio.h"

/*******************************************************************************//**
 * @brief    Macros con los pines correspontiendes a los TRIG de los sensores.
 **********************************************************************************/
#define TRIG_FRONT_PORT     0
#define TRIG_FRONT_PIN      0
#define TRIG_REAR_PORT      0
#define TRIG_REAR_PIN       1

/*******************************************************************************//**
 * @brief    Macros con el periodo de los pulsos TRIG y su duración.
 **********************************************************************************/
#define TRIG_PERIOD_MS         60
#define TRIG_DURATION_MS       1

/*******************************************************************************//**
 * @brief    Macros con los pines correspontiendes a los ECHO de los sensores.
 **********************************************************************************/
#define ECHO_FRONT_PORT     1
#define ECHO_FRONT_PIN_NUM  26
#define ECHO_REAR_PORT      1
#define ECHO_REAR_PIN_NUM   27

/*******************************************************************************//**
 * @brief    Macros con la máscara para la lectura única del respectivo pín ECHO
 *******************************************************************************/
#define ECHO_FRONT_MASK     (1 << 26)
#define ECHO_REAR_MASK      (1 << 27)

/*******************************************************************************//**
 * @brief    Enum de los identificadores de los sensores.
 **********************************************************************************/
typedef enum
{
    SENSOR_FRONT,
    SENSOR_REAR
} sensor_id;

/*******************************************************************************//**
 * @brief    Inicializa el Timer0 (captura) y los pines TRIG/ECHO del HC-SR04.
 * @details  Configura los pines TRIG y ECHO de ambos sensores, Timer0 en modo
 * 			 capture para medir los pulsos ECHO y su interrupción en el NVIC.
 * @note     USW.2.1.7.1 / 2.1.20.1
 **********************************************************************************/
void HCSR04_Init(void);

/*******************************************************************************//**
 * @brief    Convierte el tiempo del pulso ECHO a distancia en centímetros.
 * @note     USW.2.1.7.1 / 2.1.20.1
 *
 * @param time_us  Duración del pulso ECHO en microsegundos.
 * @return         Distancia en centímetros.
 **********************************************************************************/
uint32_t HCSR04_GetDistanceCm(uint32_t time_us);

/*******************************************************************************//**
 * @brief    Genera los pulsos TRIG de ambos sensores.
 * @details  Envía los pulsos TRIG a ambos sensores cada TRIG_PERIOD_MS durante un
 * 			 tiempo TRIG_DURATION_MS para realizar la lectura de la distancia.
 * @note     USW.2.1.7.1 / 2.1.20.1
 **********************************************************************************/
void HCSR04_TriggerUpdate(void);

#endif
