#ifndef BUZZER_H
#define BUZZER_H

#include "gpio.h"
#include "dma.h"
#include "timer.h"

/* @brief Selector del Buzzer*/
typedef enum
{
	BUZZER_FRONT,
	BUZZER_REAR
}buzzer_id;

/* @brief Estado del Buzzer */
typedef enum
{
    BUZZER_MODE_OFF,
    BUZZER_MODE_BEEP,
    BUZZER_MODE_CONTINUOUS
} buzzer_mode;

/*Struct con todos los datos necesarios de los buzzer */
typedef struct
{
	buzzer_id id;		/* Identificador del Buzzer.
						 *	Debe ser: BUZZER_FRONT o BUZZER_REAR */

    buzzer_mode mode;	/* Modo en el que se encuentra el buzzer.
    					 * 	Debe ser:
    					 * 				-BUZZER_MODE_OFF
    					 * 				-BUZZER_MODE_BEEP
    					 * 				-BUZZER_MODE_CONTINUOUS */

    uint8_t outputState;	/* Indica en qué estado se encuentra el transistor del buzzer
    						 * 	Debe ser:
    						 * 			- 0 -> El Buzzer no recibe la señal
    						 * 			- 1 -> El Buzzer recibe la señal */

    uint32_t threshold;		/*	Define la frecuencia con la que sonarán los pitidos .
     	 	 	 	 	 	 *	Debe ser un valor mayor a 0.
     	 	 	 	 	 	 */

    uint32_t counter;		/*Contador interno del buzzer para comparar con treshold.
    						 *y verificar si es momento de permitir o no el paso de la señal */

    uint8_t gpioPort;		/* Port en el que se encuentra el buzzer */

    uint8_t gpioPin;		/* Pin en el que se encuentra el buzzer */

} buzzer_t;


/*	Configura el Timer1 como base de tiempo para los Buzzer (1ms).
 * 	Además, configura inicialmente buzzer_front y buzzer_rear en
 * 	modo activo (BEEP), con contador en 0.
 * 	Treshold inicial para cada uno de 500 ms (pitido cada 500ms)
 */
void Buzzer_Init(void);


/* 	Configura el volumen de los buzzer.
 * 	NOTA: 	En caso de que ambos buzzer deban sonar, el buzzer con mayor volumen
 * 			(y por ende, indicador de mayor peligro) solapará el volumen del
 * 			buzzer con menor volumen.
 *
 * 	@param volume: Define el volumen de los buzzer.
 * 		   	Debe ser un valor entre 0 y 100.
 */
void Buzzer_SetVolume(uint8_t volume);

/* 	Actualiza la frecuencia del pitido
 *
 *	@param id: Define a qué buzzer se le modificará el pitido.
 *			Debe ser BUZZER_FRONT o BUZZER_REAR.
 *	@param rate: Define la frecuencia del pitido (Hz).
 *			Debe ser un valor entre 1 y 100000.
 */
void Buzzer_SetBeepRate(uint8_t id, uint32_t rate);


/*	Configura el GPIO correspondiente al transistor de cada Buzzer.
 *
 *	@param id: Define a qué buzzer se le hará el toggle para el pitido.
 *	para realizar la actualización de los pitidos.
 */

void Buzzer_update(uint8_t id);


/*	Incrementa los contadores internos de los buzzer.
 */
void Buzzer_CounterIncrement(void);


/*	Actualiza el threshold del buzzer especificado.
 * 	@param id: Buzzer al que se le desea actualizar el threshold.
 * 			Debe ser BUZZER_FRONT o BUZZER_REAR.
 * 	@param treshold: Valor nuevo del threshold deseado.
 * 			Debe ser mayor a 0.
 */
void Buzzer_SetTreshold(uint8_t id, uint32_t threshold);

#endif
