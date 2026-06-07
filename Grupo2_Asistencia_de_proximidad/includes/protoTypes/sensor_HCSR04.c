#include "sensor_HCSR04.h"


void configSensor(){

	/*Voy a configurar timer0, canal 0, no se va a producir la interrupcion, el contador se va
	 * a resetear cada vez que alcanza el valor del match. En el pin de salida se va a produccir un toggle
	*/
	TIM_MATCHCFG_Type cfgStruct1;
	cfgStruct1 = configStructMatch(TIM_MR0_INT, 10, DISABLE, DISABLE ,ENABLE,TIM_EXTMATCH_TOGGLE);
	configTimer(LPC_TIM0,TIM_TIMER_MODE,&cfgStruct1);
	activarContadorTimer(LPC_TIM0, ENABLE);

	//configuro P1.27 como pin de captura (no olvidarme del gpio)

	configCapture(LPC_TIM0, 1, ENABLE,ENABLE, ENABLE); //FLANCO Subida reseteo contador, flanco de bajada obtengo el intervalo ente flanco y flanco
	activarContadorTimer(LPC_TIM0, ENABLE);
	ActivarNVICTimer(TIMER0);
}
