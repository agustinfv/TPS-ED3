/*******************************************************************************//**
 * @file     distancia.h
 * @brief    Servicio de medición de distancia.
 * @details  Provee la interfaz de alto nivel para el sensor ultrasónico.
 *           Oculta completamente la implementación de hardware (GPIO, Timer0,
 *           pines TRIG/ECHO) detrás de funciones semánticas.
 *           Re-exporta sensor_id para que la capa de aplicación pueda
 *           identificar los sensores sin conocer el driver.
 * @note     ESW - Capa de Servicios
 **********************************************************************************/

#ifndef DISTANCIA_H
#define DISTANCIA_H

#include <stdint.h>
#include "devices/sensor_distancia.h"

/*******************************************************************************//**
 * @brief    Inicializa el servicio de distancia.
 * @details  Configura el hardware del sensor (Timer0, pines TRIG/ECHO, NVIC).
 *           Debe llamarse una vez durante la inicialización del sistema.
 **********************************************************************************/
void DIST_Init(void);

/*******************************************************************************//**
 * @brief    Convierte el tiempo del pulso ECHO a distancia en centímetros.
 * @details  Se llama desde la ISR del Timer0 para obtener la distancia medida
 *           a partir de la duración del pulso ECHO capturado.
 *
 * @param time_us  Duración del pulso ECHO en microsegundos.
 * @return         Distancia en centímetros.
 **********************************************************************************/
uint32_t DIST_GetDistanceCm(uint32_t time_us);

/*******************************************************************************//**
 * @brief    Genera los pulsos TRIG de ambos sensores.
 * @details  Debe llamarse cada 1 ms desde la ISR del Timer1. Gestiona
 *           internamente el período y la duración del pulso TRIG.
 **********************************************************************************/
void DIST_TriggerUpdate(void);

#endif
