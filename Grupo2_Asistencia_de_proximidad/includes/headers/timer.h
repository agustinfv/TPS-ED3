#ifndef TIMER_H
#define TIMER_H

#include "lpc17xx_timer.h"

typedef enum{
	TIMER0,
	TIMER1,
	TIMER2,
	TIMER3
}TIMERS;




TIM_MATCHCFG_Type configStructMatch(TIM_INT_TYPE channel, uint32_t value, FunctionalState Int, FunctionalState Stop, FunctionalState Reset, TIM_EXTMATCH_OPT ext_match);
void configTimer(LPC_TIM_TypeDef *TIMx, TIM_MODE_OPT mode, TIM_MATCHCFG_Type *matchStruct);
void configCapture(LPC_TIM_TypeDef *Timx, uint32_t CaptureChannel, FunctionalState rising, FunctionalState falling, FunctionalState Int);
void activarContadorTimer(LPC_TIM_TypeDef *Timx, FuncionalState newState);
void ActivarNVICTimer(TIMERS timer)
void DesactivarNVICTimer(TIMERS timer);

#endif
