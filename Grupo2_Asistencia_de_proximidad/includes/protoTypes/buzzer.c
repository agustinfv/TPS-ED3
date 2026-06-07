#include "buzzer.h"

#define LUT_SIZE    32
#define DAC_CENTER  512     // Mitad de escala del DAC (10 bits)

static const uint16_t sineBase[LUT_SIZE] =	//Señal senoidal de base con 100% de amplitud
{
    512, 612, 707, 794, 866, 918, 946, 946,
    918, 866, 794, 707, 612, 512, 412, 317,
    230, 158, 106,  78,  78, 106, 158, 230,
    317, 412, 512, 612, 707, 794, 866, 918
};
static uint16_t currentLUT[LUT_SIZE];			//Señal que utilizará el DMA
static uint8_t currentVolume = 100;				//Va de 0 (silencio) a 100 (máximo volumen)
buzzer_t buzzer_front;
buzzer_t buzzer_rear;

/* Inicializa el Timer1, correspondientes a los Buzzer, que siempre
 * cuenta cada 1 ms.
 */
void Buzzer_Init(void)
{
	TIM_MATCH_Type cfgTimer1;
	cfgTimer1 = configStructMatch(TIM_MR0_INT, 1000, ENABLE, DISABLE, ENABLE, TIM_EXTMATCH_NOTHING);
	configTimer(LPC_TIMER1, TIM_TIMER_MODE, (&cfgTimer1));

	buzzer_front.gpioPort	 = 0;
	buzzer_front.gpioPin 	 = 0;
	buzzer_front.id 		 = BUZZER_FRONT;
	buzzer_front.mode		 = BUZZER_MODE_BEEP;
	buzzer_front.outputState = 0;
	buzzer_front.threshold	 = 250;
	buzzer_front.counter	 = 0;

	buzzer_rear.gpioPort	= 0;
	buzzer_rear.gpioPin 	= 1;
	buzzer_rear.id 			= BUZZER_REAR;
	buzzer_rear.mode		= BUZZER_MODE_BEEP;
	buzzer_rear.outputState	= 0;
	buzzer_rear.threshold	= 250;
	buzzer_rear.counter		= 0;
}


/*	De acuerdo al volumen, configura la amplitud de la señal. Para el volumen máximo (100),
 * 	el valor de la señal máximo es 946. Para el volumen mínimo (0), el valor de la señal es
 * 	512 constante, de forma que el buzzer no produzca sonido.
 */
void Buzzer_SetVolume(uint8_t volume)
{
    uint32_t i;

    if(volume > 100) volume = 100;
    if(volume < 0 ) volume = 0;

    currentVolume = volume;

    for(i = 0; i < LUT_SIZE; i++)
    {
        int32_t centeredSample;

        centeredSample = sineBase[i] - DAC_CENTER;

        currentLUT[i] = DAC_CENTER + (centeredSample * volume) / 100;
    }
}

/*	De acuerdo a la frecuencia deseada para el correspondiente Buzzer,
 * 	actualiza el Timer que le corresponde a ese Buzzer para definir el intervalo
 * 	entre los pitidos
 * 	Nota:	Dado que utilizaremos un toggle para producir el intervalo entre los pitidos,
 * 			necesitaremos el doble de la frecuencia deseada para respetarla, razón por la que
 * 			se la multiplica por 2 en el respectivo threshold.
 */
void Buzzer_SetBeepRate(uint8_t id, uint32_t rate)
{

    if(rate <= 0 | rate >= 100000)
    {
        return;
    }
    if(id == BUZZER_FRONT)
    {
    	buzzer_front.threshold = 1/(rate*2);
    }
    if(id == BUZZER_REAR)
    {
    	buzzer_rear.threshold = 1/(rate*2);
    }

}

/* Actualiza el buzzer correspondiente de acuerdo al estado en el que se encuentre este y la salida al transistor	*/
void Buzzer_update(uint8_t id)
{
	if(id == BUZZER_FRONT)
	{
		switch(buzzer_front.mode)
		{
		case BUZZER_MODE_OFF:
			GPIO_ClearValue(buzzer_front.gpioPort, (1<< buzzer_front.gpioPin));
			break;

		case BUZZER_MODE_BEEP:
			if(buzzer_front.outputState) GPIO_SetValue(buzzer_front.gpioPort, (1<< buzzer_front.gpioPin));
			if(!(buzzer_front.outputState)) GPIO_ClearValue((buzzer_front.gpioPort), (1<<buzzer_front.gpioPin));
			break;

		case BUZZER_MODE_CONTINUOUS:
			GPIO_SetValue(buzzer_front.gpioPort, (1<<buzzer_front.gpioPin));
			break;

		default:
			break;
		}

	}
	if(id == BUZZER_REAR)
	{
		switch(buzzer_rear.mode)
		{
		case BUZZER_MODE_OFF:
			GPIO_ClearValue(buzzer_rear.gpioPort, (1<< buzzer_rear.gpioPin));
			break;

		case BUZZER_MODE_BEEP:
			if(buzzer_rear.outputState) GPIO_SetValue(buzzer_rear.gpioPort, (1<< buzzer_rear.gpioPin));
			if(!(buzzer_rear.outputState)) GPIO_ClearValue((buzzer_rear.gpioPort), (1<<buzzer_rear.gpioPin));
			break;

		case BUZZER_MODE_CONTINUOUS:
			GPIO_SetValue(buzzer_rear.gpioPort, (1<<buzzer_rear.gpioPin));
			break;

		default:
			break;
		}

	}
}
/*	Incrementa los contadores de ambos buzzer.
 */
void Buzzer_CounterIncrement(void)
{
	buzzer_front.counter ++;
	buzzer_rear.counter ++;
}

/*	Actualiza el threshold del buzzer especificado.
 */
void Buzzer_SetTreshold(buzzer_id id, uint32_t threshold)
{
	if(threshold <= 0) return;
	if(id == BUZZER_FRONT) buzzer_front.threshold = threshold;
	if(id == BUZZER_REAR) buzzer_rear.threshold = threshold;
}

