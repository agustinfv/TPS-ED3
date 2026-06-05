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

/*	Configura MR0 y MR1 del TIMER2 con valores base para el pitido por default (2Hz)
 * 	de ambos Buzzer.
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
 *	@param MatchChannel: Define a qué buzzer se le modificará el pitido.
 *			Debe ser un valor entre 0 y 1.
 *	@param rate: Define la frecuencia del pitido.
 *			Debe ser un valor entre 1 y 100000.
 */
void Buzzer_SetBeepRate(uint8_t MatchChannel, uint32_t rate);


/*	Configura el GPIO correspondiente al transistor de cada Buzzer.
 *
 *	@param buzzer_id: Especifica con qué buzzer se está trabajando.
 *			Debe ser BUZZER_FRONT o BUZZER_REAR
 *	@param buzzer_mode: Especifica en qué estado debe estar el buzzer.
 *			Debe ser:
 *				-BUZZER_MODE_OFF
 *				-BUZZER_MODE_BEEP
 *				-BUZZER_MODE_CONTINUOUS
 *	@param outputState: Indica en qué nivel lógico se encuentra el pin GPIO
 *						correspondiente al transistor del Buzzer especificado.
 *						NOTA: Solo importa en modo BUZZER_MODE_BEEP
 *			debe ser un valor entre 0 y 1. Caso contrario, será ignorado.
 */
void Buzzer_update(uint8_t buzzer_id, uint8_t buzzer_mode, uint8_t outputState);


#endif
