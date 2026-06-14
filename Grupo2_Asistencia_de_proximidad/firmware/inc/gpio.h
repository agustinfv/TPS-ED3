/*******************************************************************************//**
 * @file     gpio.h
 * @brief    Driver de configuración de pines GPIO y funciones PINSEL.
 * @details  Utilizando los drivers de GPIO y PINSEL, facilita la configuración
 * 			 básica de estos con una única función, sin necesidad de desarrollar
 * 			 los struct.
 * @note     ESW.2.1.12
 **********************************************************************************/

#ifndef GPIO_H
#define GPIO_H

#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"

/*******************************************************************************//**
 * @brief    Configura un pin como GPIO con la dirección indicada.
 * @details  Para el pin elegido establece la función 0 (GPIO) en PINSEL, modo
 * 			 tristate, y lo configura como entrada o salida según lo indicado.
 * @note     USW.2.1.12.1
 *
 * @param port  Puerto del pin (0 a 4).
 * @param pin   Número de pin dentro del puerto (0 a 31).
 * @param dir   Dirección: 0 = entrada, 1 = salida.
 **********************************************************************************/
void GPIO_ConfigPin(uint8_t port, uint8_t pin, uint8_t dir);

/*******************************************************************************//**
 * @brief    Asigna una función alternativa a un pin mediante PINSEL.
 * @details  Configura la función PINSEL indicada en el pin elegido, con modo
 * 			 tristate. No modifica la dirección del pin.
 * 			 Se usa para habilitar funciones especiales como CAP0.1, CAP0.2 o
 * 			 AOUT.
 * @note     USW.2.1.12.1
 *
 * @param port  Puerto del pin (0 a 4).
 * @param pin   Número de pin dentro del puerto (0 a 31).
 * @param func  Función PINSEL a asignar (0 a 3).
 **********************************************************************************/
void PINSEL_SetPinFunc(uint8_t port, uint8_t pin, uint8_t func);

#endif
