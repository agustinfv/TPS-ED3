/*******************************************************************************//**
 * @file     buzzer.c
 * @brief    Implementación del driver de los buzzers.
 * @details  Contiene las tablas precalculadas de la onda senoidal para cada nivel
 *           de volumen y las funciones para operar en el DAC y el DMA para
 *           controlar el volumen de los pitidos.
 * @note     ESW.2.1.4 / 2.1.8
 **********************************************************************************/

#include "buzzer.h"
#include "dma.h"

/*******************************************************************************//**
 * @brief    Macro con la cantidad de muestras de cada tabla de la señal senoidal.
 **********************************************************************************/
#define LUT_SIZE    32

/*******************************************************************************//**
 * @brief    Tablas precalculadas de la onda senoidal para cada nivel de volumen.
 * @details  Cada tabla representa la onda senoidal con una amplitud proporcional
 *           a su nivel de volumen (100%, 75%, 50%, 25%, 0%) y centrada en el
 *           punto medio del DAC (512). Los valores ya están desplazados << 6
 *           para alinearse con los bits del registro DACR.
 *           Al estar calculadas en tiempo de compilación, el CPU queda liberado
 *           de cualquier operación aritmética sobre la señal en tiempo de
 *           ejecución. La tabla de 0% emite el valor central del DAC (silencio).
 **********************************************************************************/
static const uint32_t sineLUT_100[LUT_SIZE] = {
    32768, 39168, 45248, 50816, 55424, 58752, 60544, 60544,
    58752, 55424, 50816, 45248, 39168, 32768, 26368, 20288,
    14720, 10112,  6784,  4992,  4992,  6784, 10112, 14720,
    20288, 26368, 32768, 39168, 45248, 50816, 55424, 58752
};

static const uint32_t sineLUT_75[LUT_SIZE] = {
    32768, 37568, 42112, 46272, 49728, 52224, 53568, 53568,
    52224, 49728, 46272, 42112, 37568, 32768, 27968, 23424,
    19264, 15808, 13312, 11968, 11968, 13312, 15808, 19264,
    23424, 27968, 32768, 37568, 42112, 46272, 49728, 52224
};

static const uint32_t sineLUT_50[LUT_SIZE] = {
    32768, 35968, 38976, 41792, 44096, 45760, 46656, 46656,
    45760, 44096, 41792, 38976, 35968, 32768, 29568, 26560,
    23744, 21440, 19776, 18880, 18880, 19776, 21440, 23744,
    26560, 29568, 32768, 35968, 38976, 41792, 44096, 45760
};

static const uint32_t sineLUT_25[LUT_SIZE] = {
    32768, 34368, 35840, 37248, 38400, 39232, 39680, 39680,
    39232, 38400, 37248, 35840, 34368, 32768, 31168, 29696,
    28288, 27136, 26304, 25856, 25856, 26304, 27136, 28288,
    29696, 31168, 32768, 34368, 35840, 37248, 38400, 39232
};

static const uint32_t sineLUT_0[LUT_SIZE] = {
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768,
    32768, 32768, 32768, 32768, 32768, 32768, 32768, 32768
};

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
    NVIC_SetPriority(TIMER1_IRQn, 2);
    TIMER_EnableNVIC(TIMER1);
}

/*******************************************************************************//**
 * @brief    Inicializa el hardware necesario para la generación de la senoidal.
 * @details  Llama a Buzzer_ConfigHardware(), DAC_Config() y DMA_Config() para
 *           dejar lista la reproducción continua de la señal. El DMA se inicializa
 *           apuntando a sineLUT_0 para comenzar en silencio.
 * @note     USW.2.1.4.1 / 2.1.8.1
 **********************************************************************************/
void Buzzer_Init(void)
{
    Buzzer_ConfigHardware();
    DAC_Config();
    DMA_Config(sineLUT_0, LUT_SIZE);
}

/*******************************************************************************//**
 * @brief    Selecciona la tabla precalculada correspondiente al nivel de volumen.
 * @details  Mapea el valor de volumen recibido (0–100) a una de las 5 tablas LUT
 *           precalculadas (sineLUT_0, _25, _50, _75, _100) y actualiza el origen
 *           del canal DMA apuntando a dicha tabla mediante DMA_Reload(). No realiza
 *           cálculos en tiempo de ejecución sobre la señal.
 * @note     USW.2.1.4.1 / 2.1.8.1
 *
 * @param volume  Nivel de volumen deseado (0 = silencio, 100 = volumen máximo).
 **********************************************************************************/
void Buzzer_ApplyVolume(uint8_t volume)
{
    const uint32_t *lut;

    if      (volume == 0)  lut = sineLUT_0;
    else if (volume <= 25) lut = sineLUT_25;
    else if (volume <= 50) lut = sineLUT_50;
    else if (volume <= 75) lut = sineLUT_75;
    else                   lut = sineLUT_100;

    DMA_Reload(lut);
}
