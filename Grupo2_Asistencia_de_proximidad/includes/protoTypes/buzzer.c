#include "buzzer.h"

#define LUT_SIZE    32
#define DAC_CENTER  512     // Mitad de escala del DAC (10 bits)
#define BUZZER_FRONT_GPIO	(1<<0)	//Pin 0 del PORT0
#define BUZZER_REAR_GPIO	(1<<1)	//Pin 1 del PORT0

static const uint16_t sineBase[LUT_SIZE] =	//Señal senoidal de base con 100% de amplitud
{
    512, 612, 707, 794, 866, 918, 946, 946,
    918, 866, 794, 707, 612, 512, 412, 317,
    230, 158, 106,  78,  78, 106, 158, 230,
    317, 412, 512, 612, 707, 794, 866, 918
};
static uint16_t currentLUT[LUT_SIZE];			//Señal que utilizará el DMA
static uint8_t currentVolume = 100;				//Va de 0 (silencio) a 100 (máximo volumen)


/* Inicializa el Timer2, correspondientes a los Buzzer, con beep default cada 0,5s
 * para cada canal de MATCH.
 * */
void Buzzer_Init(void)
{
	TIM_MATCH_Type cfgTimer2_MR0:
	TIM_MATCH_Type cfgTimer2_MR1:
	cfgTimer2_MR0 = configStructMatch(TIM_MR0_INT, 500000, ENABLE, DISABLE, ENABLE, TIM_EXTMATCH_NOTHING);
	cfgTimer1_MR1 = configStructMatch(TIM_MR1_INT, 500000, ENABLE, DISABLE, ENABLE, TIM_EXTMATCH_NOTHING);
	configTimer(LPC_TIMER2, TIM_TIMER_MODE, (&cfgTimer2_MR0));
	configTimer(LPC_TIMER2, TIM_TIMER_MODE, (&cfgTimer2_MR1));

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
 * 	Nota:	Dado que utilizaremos el Timer como Toggle para producir el intervalo entre los pitidos,
 * 			el Timer necesita el doble de la frecuencia deseada para respetarla, razón por la que
 * 			se lo multiplica por 2 en matchValue
 */
void Buzzer_SetBeepRate(uint8_t MatchChannel, uint32_t rate)
{
    uint32_t matchValue;

    if(rate <= 0 | rate >= 100000)
    {
        return;
    }


    matchValue = (25*10^6)/(25*rate*2);

    if(MatchChannel = 0) TIM_UpdateMatchValue(LPC_TIM2,TIM_MR0_INT, matchValue);
    if(MatchChannel = 1) TIM_UpdateMatchValue(LPC_TIM2,TIM_MR1_INT, matchValue);
}

/* Actualiza el buzzer correspondiente de acuerdo al estado en el que se encuentre este	*/
void Buzzer_update(uint8_t buzzer_id, uint8_t buzzer_mode, uint8_t outputState)
{
	if(buzzer_id == BUZZER_FRONT)
	{
		switch(buzzer_mode)
		{
		case BUZZER_MODE_OFF:
			GPIO_ClearValue(0, BUZZER_FRONT_GPIO);
			break;

		case BUZZER_MODE_BEEP:
			if(outputState) GPIO_SetValue(0, BUZZER_FRONT_GPIO);
			if(!outputState) GPIO_ClearValue(0, BUZZER_FRONT_GPIO);
			break;

		case BUZZER_MODE_CONTINUOUS:
			GPIO_SetValue(0, BUZZER_FRONT_GPIO);
			break;

		default:
			break;
		}

	}
	if(buzzer_id == BUZZER_REAR)
	{
		switch(buzzer_mode)
		{
		case BUZZER_MODE_OFF:
			GPIO_ClearValue(0, BUZZER_REAR_GPIO);
			break;

		case BUZZER_MODE_BEEP:
			if(outputState) GPIO_SetValue(0, BUZZER_REAR_GPIO);
			if(!outputState) GPIO_ClearValue(0, BUZZER_REAR_GPIO);
			break;

		case BUZZER_MODE_CONTINUOUS:
			GPIO_SetValue(0, BUZZER_REAR_GPIO);
			break;

		default:
			break;
		}

	}
}


