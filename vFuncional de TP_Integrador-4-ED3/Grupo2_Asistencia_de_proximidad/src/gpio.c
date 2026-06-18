/*******************************************************************************//**
 * @file     gpio.c
 * @brief    Implementación del driver de configuración de pines GPIO y PINSEL.
 * @details  Implementa las funciones usando los drivers de NXP. Cada función
 * 			 construye internamente el struct de configuración requerido y utiliza
 * 			 el driver correspondiente para la escritura en los registros.
 * @note     ESW.2.1.12
 **********************************************************************************/

#include "gpio.h"

/*******************************************************************************//**
 * @brief    Configura un pin como GPIO con la dirección indicada.
 * @details  Construye un PINSEL_CFG_Type con función 0 (GPIO) y modo tristate,
 * 		     llama a PINSEL_ConfigPin() y luego configura la dirección del pin
 * 		     con GPIO_SetDir().
 * @note     USW.2.1.12.1
 *
 * @param port  Puerto del pin (0 a 4).
 * @param pin   Número de pin dentro del puerto (0 a 31).
 * @param dir   Dirección: 0 = entrada, 1 = salida.
 **********************************************************************************/
void GPIO_ConfigPin(uint8_t port, uint8_t pin, uint8_t dir)
{
	PINSEL_CFG_Type pinCfg;

	pinCfg.Portnum   = port;
	pinCfg.Pinnum    = pin;
	pinCfg.Funcnum   = PINSEL_FUNC_0;
	pinCfg.Pinmode   = PINSEL_PINMODE_TRISTATE;
	pinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;

	PINSEL_ConfigPin(&pinCfg);

	if(dir)
		GPIO_SetDir(port, (1 << pin), 1);
	else
		GPIO_SetDir(port, (1 << pin), 0);
}

/*******************************************************************************//**
 * @brief    Asigna una función alternativa a un pin mediante PINSEL.
 * @details  Construye un PINSEL_CFG_Type con la función indicada, en modo tristate
 *           y llama a PINSEL_ConfigPin(). No modifica la dirección del pin.
 *           Usada para habilitar CAP0.0, CAP0.1 y AOUT.
 * @note     USW.2.1.12.1
 *
 * @param port  Puerto del pin (0 a 4).
 * @param pin   Número de pin dentro del puerto (0 a 31).
 * @param func  Función PINSEL a asignar (0 a 3).
 **********************************************************************************/
void PINSEL_SetPinFunc(uint8_t port, uint8_t pin, uint8_t func)
{
	PINSEL_CFG_Type pinCfg;

	pinCfg.Portnum   = port;
	pinCfg.Pinnum    = pin;
	pinCfg.Funcnum   = func;
	pinCfg.Pinmode   = PINSEL_PINMODE_TRISTATE;
	pinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;

	PINSEL_ConfigPin(&pinCfg);
}
