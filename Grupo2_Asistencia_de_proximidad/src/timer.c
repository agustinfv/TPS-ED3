/*******************************************************************************//**
 * @file     timer.c
 * @brief    Implementación del driver de inicialización y control de timers.
 * @details  Implementa las funciones para configurar los timers del LPC1769,
 * 			 utilizando los drivers de NXP.
 * 			 Configura el prescaler fijo de 1 µs para usar directamente los
 *           valores de match y capture en microsegundos. El Timer0 se utiliza como
 *           base de tiempo para los canales de capture del HC-SR04, y el Timer1
 *           como base de tiempo de 1 ms para el control de buzzers y pulso TRIG.
 * @note     ESW.2.1.11
 **********************************************************************************/

#include "timer.h"

/*******************************************************************************//**
 * @brief    Construye y retorna una estructura TIM_MATCHCFG_Type.
 * @details  Asigna cada campo del struct de los drivers con el parámetro
 * 			 correspondiente.
 *           El valor de match se guarda como value - 1 para ajustar el ciclo
 *           extra del contador (match = 25? value - 1 = 24 -> 0-24 = 25) y
 *           lograr un período exacto.
 * @note     USW.2.1.11.1
 *
 * @param channel    Canal de match (TIM_MR0_INT a TIM_MR3_INT).
 * @param value      Valor del match en µs.
 * @param Int        ENABLE o DISABLE: genera interrupción al hacer match.
 * @param Stop       ENABLE o DISABLE: detiene el contador al hacer match.
 * @param Reset      ENABLE o DISABLE: resetea el contador al hacer match.
 * @param ext_match  Acción sobre el pin externo al hacer match.
 * @return           Struct TIM_MATCHCFG_Type inicializada.
 **********************************************************************************/
TIM_MATCHCFG_Type TIMER_ConfigStructMatch(TIM_INT_TYPE channel, uint32_t value,
		FunctionalState Int, FunctionalState Stop, FunctionalState Reset,
		TIM_EXTMATCH_OPT ext_match)
{
	TIM_MATCHCFG_Type cfgMatchX;

	cfgMatchX.MatchChannel       = channel;
	cfgMatchX.MatchValue         = value - 1;
	cfgMatchX.IntOnMatch         = Int;
	cfgMatchX.StopOnMatch        = Stop;
	cfgMatchX.ResetOnMatch       = Reset;
	cfgMatchX.ExtMatchOutputType = ext_match;

	return cfgMatchX;
}

/*******************************************************************************//**
 * @brief    Inicializa un timer con prescaler de 1 µs y configura el match indicado.
 * @details  Construye un TIM_TIMERCFG_Type con prescaler en microsegundos (valor 1),
 *           Utiliza TIM_Init() y TIM_ConfigMatch() para configurar el match
 *           especificado. Termina con TIM_Cmd(DISABLE) para que el contador quede
 *           detenido hasta cuando se quiera habilitarlo.
 * @note     USW.2.1.11.1
 *
 * @param TIMx         Puntero al timer: LPC_TIM0 – LPC_TIM3.
 * @param mode         Modo de operación del timer (TIM_TIMER_MODE, etc.).
 * @param matchStruct  Puntero al struct del match ya configurado.
 **********************************************************************************/
void TIMER_Config(LPC_TIM_TypeDef *TIMx, TIM_MODE_OPT mode,
		TIM_MATCHCFG_Type *matchStruct)
{
	TIM_TIMERCFG_Type cfgTimer;

	cfgTimer.PrescaleOption = TIM_PRESCALE_USVAL;
	cfgTimer.PrescaleValue  = 1;

	TIM_Init(TIMx, mode, &cfgTimer);
	TIM_ConfigMatch(TIMx, matchStruct);
	TIM_Cmd(TIMx, DISABLE);
}

/*******************************************************************************//**
 * @brief    Configura un canal de capture en un timer ya inicializado.
 * @details  Construye un TIM_CAPTURECFG_Type y llama a TIM_ConfigCapture().
 *           Utiliza TIM_ResetCounter() para asegurarse de arrancar el contador
 *           desde cero.
 * @note     USW.2.1.11.1
 *
 * @param TIMx           Puntero al timer: LPC_TIM0 – LPC_TIM3.
 * @param CaptureChannel Canal de capture (0 o 1).
 * @param rising         ENABLE o DISABLE: capture en flanco de subida.
 * @param falling        ENABLE o DISABLE: capture en flanco de bajada.
 * @param Int            ENABLE o DISABLE: genera interrupción en cada capture.
 **********************************************************************************/
void TIMER_ConfigCapture(LPC_TIM_TypeDef *TIMx, uint32_t CaptureChannel,
		FunctionalState rising, FunctionalState falling, FunctionalState Int)
{
	TIM_CAPTURECFG_Type cfgCapture;

	cfgCapture.CaptureChannel = CaptureChannel;
	cfgCapture.RisingEdge     = rising;
	cfgCapture.FallingEdge    = falling;
	cfgCapture.IntOnCaption   = Int;

	TIM_ConfigCapture(TIMx, &cfgCapture);
	TIM_ResetCounter(TIMx);
}

/*******************************************************************************//**
 * @brief    Habilita o deshabilita el contador de un timer.
 * @details  Utiliza directamente de los drivers a TIM_Cmd().
 * @note     USW.2.1.11.1
 *
 * @param TIMx      Puntero al timer: LPC_TIM0 – LPC_TIM3.
 * @param newState  ENABLE o DISABLE: inicia el contador o lo detiene.
 **********************************************************************************/
void TIMER_EnableCounter(LPC_TIM_TypeDef *TIMx, FunctionalState newState)
{
	TIM_Cmd(TIMx, newState);
}

/*******************************************************************************//**
 * @brief    Habilita la interrupción NVIC del timer indicado.
 * @details  Utiliza el enum TIMERS para elegir el IRQn correspondiente y llama
 * 			 a NVIC_EnableIRQ() para activar la atención de la interrupción del
 * 			 timer indicado.
 * @note     USW.2.1.11.1
 *
 * @param timer  Timer a habilitar en el NVIC: TIMER0 – TIMER3.
 **********************************************************************************/
void TIMER_EnableNVIC(TIMERS timer)
{
	switch(timer)
	{
	case TIMER0:
		NVIC_EnableIRQ(TIMER0_IRQn);
		break;
	case TIMER1:
		NVIC_EnableIRQ(TIMER1_IRQn);
		break;
	case TIMER2:
		NVIC_EnableIRQ(TIMER2_IRQn);
		break;
	case TIMER3:
		NVIC_EnableIRQ(TIMER3_IRQn);
		break;
	default:
		break;
	}
}

/*******************************************************************************//**
 * @brief    Deshabilita la interrupción NVIC del timer indicado.
 * @details  Utiliza el enum TIMERS para elegir el IRQn correspondiente y llama
 * 			 a NVIC_DisableIRQ() para desactivar la atención de la interrupción
 * 			 del timer indicado.
 * @note     USW.2.1.11.1
 *
 * @param timer  Timer a deshabilitar en el NVIC: TIMER0 – TIMER3.
 **********************************************************************************/
void TIMER_DisableNVIC(TIMERS timer)
{
	switch(timer)
	{
	case TIMER0:
		NVIC_DisableIRQ(TIMER0_IRQn);
		break;
	case TIMER1:
		NVIC_DisableIRQ(TIMER1_IRQn);
		break;
	case TIMER2:
		NVIC_DisableIRQ(TIMER2_IRQn);
		break;
	case TIMER3:
		NVIC_DisableIRQ(TIMER3_IRQn);
		break;
	default:
		break;
	}
}
