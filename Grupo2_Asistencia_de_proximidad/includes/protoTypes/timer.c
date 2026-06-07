#include "timer.h"

/*
 * * @param[in]	 channel Timer channel, debe ser:
 					TIM_MR0_INT =0, /*!< interrupt for Match channel 0
					TIM_MR1_INT =1, /*!< interrupt for Match channel 1
					TIM_MR2_INT =2, /*!< interrupt for Match channel 2
					TIM_MR3_INT =3, /*!< interrupt for Match channel 3
					TIM_CR0_INT =4, /*!< interrupt for Capture channel 0
					TIM_CR1_INT =5, /*!< interrupt for Capture channel 1

 * @param[in]
 *                     - value: 			time for interruption
 *                     - IntOnMatch     : if SET, interrupt will be generated when MRxx match the value in TC
 *                     - StopOnMatch     : if SET, TC and PC will be stopped whenM Rxx match the value in TC
 *                     - ResetOnMatch : if SET, Reset on MR0 when MRxx match the value in TC
 *                     - ext_match: Select output for external match
 *                         +      0:    Do nothing for external output pin if match
 *                         +  	  1:    Force external output pin to low if match
 *                         +      2:	    Force external output pin to high if match
 *                         +      3:    Toggle external output pin if match

*/
TIM_MATCHCFG_Type configStructMatch(TIM_INT_TYPE channel, uint32_t value, FunctionalState Int, FunctionalState Stop, FunctionalState Reset, TIM_EXTMATCH_OPT ext_match){
	TIM_MATCHCFG_Type cfgMatchX;

		cfgMatchX.MatchChannel = channel;
		cfgMatchX.MatchValue = value - 1; //int cada ... (prescalador en 1microSegundo)
		cfgMatchX.IntOnMatch = Int;
		cfgMatchX.StopOnMatch = Stop;
		cfgMatchX.ResetOnMatch = Reset;
		cfgMatchX.ExtMatchOutputType = ext_match;

	return cfgMatchX;
}

/*
 * Preescalador en 1 microSegundo
 *
 * @param[in]    TIMx  Timer selection, debe ser:
                   - LPC_TIM0: TIMER0 peripheral
                   - LPC_TIM1: TIMER1 peripheral
                   - LPC_TIM2: TIMER2 peripheral
                   - LPC_TIM3: TIMER3 peripheral


 * @param[in]	mode Timer counter mode, debe ser:
  				   -TIM_TIMER_MODE = 0,				 Timer mode
				   -TIM_COUNTER_RISING_MODE,		 Counter rising mode
				   -TIM_COUNTER_FALLING_MODE,		 Counter falling mode
				   -TIM_COUNTER_ANY_MODE			Counter on both edges
 * @param[in]   TIM_MatchConfigStruct Pointer to TIM_MATCHCFG_Type

*/
void configTimer(LPC_TIM_TypeDef *TIMx, TIM_MODE_OPT mode, TIM_MATCHCFG_Type *matchStruct){
	TIM_TIMERCFG_Type cfgTimer;

	//Config timer
	cfgTimer.PrescaleOption = TIM_PRESCALE_USVAL;//int en uS
	cfgTimer.PrescaleValue = 1; //int cada 1uS, funcion init se encarga de restarle el -1

	//Pre habilitacion timer0.0
	TIM_Init(TIMx,mode,&cfgTimer);
	TIM_ConfigMatch(TIMx,matchStruct);
	TIM_Cmd(TIMx,DISABLE);


}


/*
 * @param[in]    TIMx  Timer selection, debe ser:
                   - LPC_TIM0: TIMER0 peripheral
                   - LPC_TIM1: TIMER1 peripheral
                   - LPC_TIM2: TIMER2 peripheral
                   - LPC_TIM3: TIMER3 peripheral

 * @param[in]	 CaptureChannel, debe ser un valor entre 0 y 1

 * @param[in] 	 rising, debe ser:
  					ENABLE: Activa flanco de subida
  					DISABLE: Desactiva esta funcion
 * @param[in] 	 falling, debe ser:
  					ENABLE: Activa flanco de bajada
  					DISABLE: Desactiva esta funcion
 * @param[in] 	 Int, debe ser:
  					ENABLE: Activa la interrupcion
  					DISABLE: Desactiva esta funcion

 */
void configCapture(LPC_TIM_TypeDef *Timx, uint32_t CaptureChannel, FunctionalState rising, FunctionalState falling, FunctionalState Int){
	TIM_TIMERCFG_Type cfgTimerCapture;
	TIM_CAPTURECFG_Type cfgCapture;

	cfgTimerCapture.PrescaleOption = TIM_PRESCALE_USVAL;
	cfgTimerCapture.PrescaleValue = 1; //cuenta cada 1uS

	cfgCapture.CaptureChannel = CaptureChannel;
	cfgCapture.RisingEdge = rising;
	cfgCapture.FallingEdge = falling;
	cfgCapture.IntOnCaption = Int;

	TIM_Init(Timx, TIM_TIMER_MODE, &cfgTimerCapture);
	TIM_ConfigCapture(TIMx, &cfgCapture);
	TIM_Cmd(Timx,DISABLE);

	TIM_ResetCounter(Timx);


}
/*
 * @param[in]    TIMx  Timer selection, debe ser:
                   - LPC_TIM0: TIMER0 peripheral
                   - LPC_TIM1: TIMER1 peripheral
                   - LPC_TIM2: TIMER2 peripheral
                   - LPC_TIM3: TIMER3 peripheral

 *@param[in]	newState, activa el contador para ese determinado Timx, debe ser
				 - ENABLE
				 - DISABLE

*/
void activarContadorTimer(LPC_TIM_TypeDef *Timx, FuncionalState newState){
	{
		TIM_Cmd(Timx, newState);
	}

}

/*
 * Se encarga habilitar las interrupcion
 *
 * @param [in] timer, debe ser:
  				 - TIMER0
  				 - TIMER1
  				 - TIMER2
  				 - TIMER3
 */
void ActivarNVICTimer(TIMERS timer){
	switch(timer){
	case 0:
		NVIC_EnableIRQn(TIMER0_IRQn);
		break;
	case 1:
		NVIC_EnableIRQn(TIMER1_IRQn);
		break;
	case 2:
		NVIC_EnableIRQn(TIMER2_IRQn);
		break;
	case 3:
		NVIC_EnableIRQn(TIMER3_IRQn);
		break;
	}
}

/*
 * Se encarga deshabilitar la interrupcion
 *
 * @param [in] timer, debe ser:
  				 - TIMER0
  				 - TIMER1
  				 - TIMER2
  				 - TIMER3
 */
void DesactivarNVICTimer(TIMERS timer){
	switch(timer){
	case 0:
		NVIC_DisableIRQn(TIMER0_IRQn);
		break;
	case 1:
		NVIC_DisableIRQn(TIMER1_IRQn);
		break;
	case 2:
		NVIC_DisableIRQn(TIMER2_IRQn);
		break;
	case 3:
		NVIC_DisableIRQn(TIMER3_IRQn);
		break;
	}
}



/*
 * Mi idea en el main es
 * TIM_MATCH_Type cfgStruct:
 * cfgStruct = configStructMatch(TIM_MR0_INT, 1000, ENABLE, DISABLE, ENABLE, TIM_EXTMATCH_NOTHING);
 *
 * configTimer(LPC_TIMER0, TIM_TIMER_MODE, (&cfgStruct))
 *
 *  y listo, el timer0 canal 0
 */

