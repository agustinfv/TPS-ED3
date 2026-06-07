#include "hcsr04.h"

#define SENSOR_CONST 0.017	/*	Constante a multiplicar por el tiempo en us obtenido para obtener la distancia en cm.
								El cálculo es (340(m/s)/2)*(100cm/1m)*(1s/(1*10^6 us)). El 2 por la ida y vuelta del sonido
							*/

sensor_t sensor_front;
sensor_t sensor_rear;


/*	Inicializa ambos sensores con valores default.
 */
void HCSR04_Init(void)
{
	sensor_front.id = SENSOR_FRONT;
	sensor_front.state = SAFE_ZONE;
	sensor_front.distance = 100;

	sensor_rear.id = SENSOR_FRONT;
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
