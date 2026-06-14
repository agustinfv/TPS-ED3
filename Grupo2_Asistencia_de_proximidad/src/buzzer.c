/*******************************************************************************//**
 * @file     buzzer.c
 * @brief    Implementación del driver de los buzzers.
 * @details  Contiene la tabla base de la onda senoidal y las funciones para
 *           operar en el DAC y el DMA para controlar el volumen de los pitidos.
 * @note     ESW.2.1.4 / 2.1.8
 **********************************************************************************/

#include "buzzer.h"

/*******************************************************************************//**
 * @brief    Macros con el tamaño de la señal y el centro del DAC
 * @details  DAC_CENTER utilizado para cálculos. Es la mitad de resolución del DAC.
 **********************************************************************************/
#define LUT_SIZE    32
#define DAC_CENTER  512

static const uint16_t sineBase[LUT_SIZE] =
{
    512, 612, 707, 794, 866, 918, 946, 946,
    918, 866, 794, 707, 612, 512, 412, 317,
    230, 158, 106,  78,  78, 106, 158, 230,
    317, 412, 512, 612, 707, 794, 866, 918
};
/*******************************************************************************//**
 * @brief    Array con la señal a sacar por el DAC.
 **********************************************************************************/
static uint32_t currentLUT[LUT_SIZE];

/*******************************************************************************//**
 * @brief    Configura el Timer1 como base de tiempo de 1 ms.
 * @note     USW.2.1.4.1 / 2.1.8.1
 **********************************************************************************/
static void Buzzer_ConfigHardware(void)
{
    TIM_MATCHCFG_Type cfgTimer1;

    cfgTimer1 = TIMER_ConfigStructMatch(TIM_MR0_INT, 1000, ENABLE, DISABLE,
            ENABLE, TIM_EXTMATCH_NOTHING);
    TIMER_Config(LPC_TIM1, TIM_TIMER_MODE, &cfgTimer1);
    TIMER_EnableCounter(LPC_TIM1, ENABLE);
    TIMER_EnableNVIC(TIMER1);
}

/*******************************************************************************//**
 * @brief    Inicializa el hardware necesario para la generación de la senoidal.
 * @details  Llama a Buzzer_ConfigHardware(), DAC_Config() y
 *           DMA_Config() para dejar lista la reproducción continua de la señal.
 * @note     USW.2.1.4.1 / 2.1.8.1
 **********************************************************************************/
void Buzzer_Init(void)
{
    Buzzer_ConfigHardware();
    DAC_Config();
    Buzzer_ApplyVolume(0);
    DMA_Config(currentLUT, LUT_SIZE);
}

/*******************************************************************************//**
 * @brief    Ajusta el volumen de la señal modificando su amplitud.
 * @details  Calcula los nuevos valores de cada muestra de la señal de acuerdo al
 * 			 volumen indicado y los desplaza << 6 lugares para el registro DACR.
 * 			 Inicialmente se desactiva el canal de transferencia de la señal para
 * 			 evitar problemas, y al final vuelve a activarse.
 * @note     USW.2.1.4.1 / 2.1.8.1
 *
 * @param volume  Nivel de volumen deseado (0 = silencio, 100 = volumen máximo).
 **********************************************************************************/
void Buzzer_ApplyVolume(uint8_t volume)
{
    int32_t sample;

    GPDMA_ChannelCmd(DMA_CHANNEL_DAC, DISABLE);

    if(volume > 100) volume = 100;

    for(uint8_t i = 0; i < LUT_SIZE; i++)
    {
        sample        = (int32_t)sineBase[i] - DAC_CENTER;
        sample        = DAC_CENTER + (sample * volume) / 100;
        currentLUT[i] = ((uint32_t)sample << 6);
    }

    GPDMA_ChannelCmd(DMA_CHANNEL_DAC, ENABLE);
}
