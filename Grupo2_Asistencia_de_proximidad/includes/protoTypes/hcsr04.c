#include "hcsr04.h"

#define SENSOR_CONST 0.017	/*	Constante a multiplicar por el tiempo en us obtenido para obtener la distancia en cm.
								El cálculo es (340(m/s)/2)*(100cm/1m)*(1s/(1*10^6 us)). El 2 por la ida y vuelta del sonido
							*/

sensor_t sensor_front;
sensor_t sensor_rear;


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

/*	Inicializa ambos sensores con valores default.
 */
void HCSR04_Init(void)
{
	sensor_front.id = SENSOR_FRONT;
	sensor_front.state = SAFE_ZONE;
	sensor_front.distance = 100;

	sensor_rear.id = SENSOR_REAR;
	sensor_rear.state = SAFE_ZONE;
	sensor_rear.distance = 100;
}

/*	Toma el valor del tiempo que el pulso ECHO estuvo activo
 *	y lo multiplioa por SENSOR_CONST para obtener la distancia.
 *	Esa distancia se asigna al correspondiente sensor indicado en id.
 */
void HCSR04_SetDistanceCm(uint8_t id, uint32_t time_us)
{
	if(id == SENSOR_FRONT)
	{
		sensor_front.distance = SENSOR_CONST*time_us;
	}
	if(id == SENSOR_REAR)
	{
		sensor_rear.distance = SENSOR_CONST*time_us;
	}

}
