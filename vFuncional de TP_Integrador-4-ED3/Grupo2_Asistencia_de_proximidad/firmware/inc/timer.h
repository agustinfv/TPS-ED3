/*******************************************************************************//**
 * @file     timer.h
 * @brief    Driver de inicialización y control de los timers del LPC1769.
 * @details  Muestra las funciones (utilizando los drivers de NXP)
 *           para configurar el modo de operación, los canales de match, los
 *           canales de capture, y el habilitado/deshabilitado de los
 *           contadores y sus interrupciones en el NVIC.
 *           Prescaler fijo de 1 µs en todas las configuraciones de timer.
 * @note     ESW.2.1.11
 **********************************************************************************/

#ifndef TIMER_H
#define TIMER_H

#include "lpc17xx_timer.h"

/*******************************************************************************//**
 * @brief    Enum de los cuatro timers del LPC1769.
 **********************************************************************************/
typedef enum
{
	TIMER0,
	TIMER1,
	TIMER2,
	TIMER3
} TIMERS;

/*******************************************************************************//**
 * @brief    Construye y retorna una estructura TIM_MATCHCFG_Type con los parámetros
 *           dados.
 * @details  Facilita la construcción del struct de match sin necesidad de completar
 *           cada campo de este manualmente. El valor de match se ajusta
 *           automáticamente restando 1 para ajustar el ciclo extra del contador.
 * @note     USW.2.1.11.1
 *
 * @param channel    Canal de match (TIM_MR0_INT a TIM_MR3_INT).
 * @param value      Valor del match en µs (con prescaler de 1 µs).
 * @param Int        ENABLE o DISABLE: genera interrupción al hacer match.
 * @param Stop       ENABLE o DISABLE: detiene el contador al hacer match.
 * @param Reset      ENABLE o DISABLE: resetea el contador al hacer match.
 * @param ext_match  Acción sobre el pin externo al hacer match.
 * @return           Struct TIM_MATCHCFG_Type inicializada.
 **********************************************************************************/
TIM_MATCHCFG_Type TIMER_ConfigStructMatch(TIM_INT_TYPE channel, uint32_t value,
		FunctionalState Int, FunctionalState Stop, FunctionalState Reset,
		TIM_EXTMATCH_OPT ext_match);

/*******************************************************************************//**
 * @brief    Inicializa un timer en el modo indicado con prescaler de 1 µs y
 *           configura el canal de match indicado.
 * @details  Utiliza TIM_Init() y TIM_ConfigMatch() para configurar el match
 * 			 indicado. El contador queda deshabilitado inicialmente,
 *           debe habilitarse luego con activarContadorTimer().
 * @note     USW.2.1.11.1
 *
 * @param TIMx         Puntero al timer: LPC_TIM0 – LPC_TIM3.
 * @param mode         Modo de operación del timer (TIM_TIMER_MODE, etc.).
 * @param matchStruct  Puntero al struct del match ya construido.
 **********************************************************************************/
void TIMER_Config(LPC_TIM_TypeDef *TIMx, TIM_MODE_OPT mode,
		TIM_MATCHCFG_Type *matchStruct);

/*******************************************************************************//**
 * @brief    Configura un canal de capture en un timer ya inicializado.
 * @details  Debe llamarse después de configTimer() con el mismo parámetro TIMx.
 *           Utiliza TIM_ConfigCapture() y luego resetea el contador con
 *           TIM_ResetCounter() para asegurarse de que comience en cero.
 * @note     USW.2.1.11.1
 *
 * @param TIMx           Puntero al timer: LPC_TIM0 – LPC_TIM3.
 * @param CaptureChannel Canal de capture (0 o 1).
 * @param rising         ENABLE o DISABLE: captura en flanco de subida.
 * @param falling        ENABLE o DISABLE: captura en flanco de bajada.
 * @param Int            ENABLE o DISABLE: genera interrupción en cada captura.
 **********************************************************************************/
void TIMER_ConfigCapture(LPC_TIM_TypeDef *TIMx, uint32_t CaptureChannel,
		FunctionalState rising, FunctionalState falling, FunctionalState Int);

/*******************************************************************************//**
 * @brief    Habilita o deshabilita el contador de un timer.
 * @details  Utiliza directamente de los drivers a TIM_Cmd()
 * @note     USW.2.1.11.1
 *
 * @param TIMx      Puntero al timer: LPC_TIM0 – LPC_TIM3.
 * @param newState  ENABLE o DISABLE: inicia el contador o lo detiene.
 **********************************************************************************/
void TIMER_EnableCounter(LPC_TIM_TypeDef *TIMx, FunctionalState newState);

/*******************************************************************************//**
 * @brief    Habilita la interrupción NVIC del timer indicado.
 * @details  Utiliza NVIC_EnableIRQ() para habilitar la atención de la interrupción
 * 			 del timer indicado.
 * @note     USW.2.1.11.1
 *
 * @param timer  Timer a habilitar en el NVIC: TIMER0 – TIMER3.
 **********************************************************************************/
void TIMER_EnableNVIC(TIMERS timer);

/*******************************************************************************//**
 * @brief    Deshabilita la interrupción NVIC del timer indicado.
 * @details  Utiliza NVIC_DisableIRQ() para deshabilitar la atención de la
 * 			 interrupción del timer indicado.
 * @note     USW.2.1.11.1
 *
 * @param timer  Timer a deshabilitar en el NVIC: TIMER0 – TIMER3.
 **********************************************************************************/
void TIMER_DisableNVIC(TIMERS timer);

#endif
