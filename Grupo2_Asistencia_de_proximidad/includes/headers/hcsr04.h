#ifndef HCSR04_H
#define HCSR04_H

#define ECHO_FRONT_PIN (1<<26)		//Pin ECHO del sensor frontal  en el 1.26 (CAP0.0)
#define ECHO_REAR_PIN (1<<27)		//Pin ECHO del sensor trasero  en el 1.27 (CAP0.1

/*	Identificación de los sensores.
 */
typedef enum
{
	SENSOR_FRONT,
	SENSOR_REAR
}sensor_id;

/*	Estados que pueden tomar los sensores.
 */
typedef enum
{
	SAFE_ZONE,
	WARNING_ZONE,
	DANGEROUS_ZONE
}sensor_state;

/*	Struct de los sensores con los datos necesarios para operar con ellos.
 */
typedef struct
{
	sensor_id	 id;	/*	Identificación del sensor.
						 * 	Debe ser SENSOR_FRONT o SENSOR_REAR. */

	sensor_state state;	/*	Estados que puede tomar el sensor.
						 *	Debe ser:
						 *			-SAFE_ZONE
						 *			-WARNING_ZONE
						 *			-DANGEROUS_ZONE */

	uint32_t	 distance;	/*	Distancia a la que se encuentra el objeto
							 *	del respectivo sensor. */
}sensor_t;

/*	Inicializa con valores default ambos sensores: el frontal y trasero.
 * 	Ambos comienzan en SAFE_ZONE, cada uno con su propia ID y distance = 100.
 */
void HCSR04_Init(void);

/*	Establece la distancia a la que se encuentra el objeto del sensor
 * 	@param id: especificación de a qué sensor se le quiere asignar la distancia.
 * 			Debe ser SENSOR_FRONT o SENSOR_REAR
 * 	@param time_us: Tiempo en el que el pulso ECHO enviado por el sensor estuvo en alto
 * 				Puede ser cualquier valor entre 0 y (2^32 -1)
 */
void HCSR04_SetDistanceCm(uint8_t id, uint32_t time_us);

void proximity_Assistant(void);

#endif
