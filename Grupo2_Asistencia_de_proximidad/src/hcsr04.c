/*******************************************************************************//**
 * @file     hcsr04.c
 * @brief    Implementación del driver del sensor del HC-SR04.
 * @details  Implementa la inicialización del Timer0 en modo capture, la generación
 *           de los pulsos TRIG y la conversión de tiempo de ECHO a
 *           distancia.
 * @note     ESW.2.1.7 / 2.1.20
 **********************************************************************************/

#include "hcsr04.h"

/*******************************************************************************//**
 * @brief    Macros con las constantes para el cálculo de la distancia
 * @details  La distancia se calcula de la siguiente manera:
 * 			 Velocidad del sonido: aproximadamente 340 m/s.
 *    		 d (cm) = ((340 m/s) / 2) * (100 cm/m) * (1 s / 10^6 µs) * time_us
 *           = 0.017 (cm/µs) * time_us
 *  		 Pero como tratamos con enteros: d = (time_us * 17) / 1000
 **********************************************************************************/
#define SENSOR_CONST_NUM    17
#define SENSOR_CONST_BACK   1000

/*******************************************************************************//**
 * @brief    Inicializa el Timer0 para capture y los pines TRIG/ECHO del HC-SR04.
 * @details  Configura los pines TRIG (P0.0 y P0.1), ECHO (CAP0.0 y CAP0.1) de
 * 			 los respectivos sensores y Timer0 en modo capture con interrupciones
 * 			 por ambos flancos
 * @note     USW.2.1.7.1 / 2.1.20.1
 **********************************************************************************/
static void HCSR04_ConfigHardware(void)
{
    TIM_MATCHCFG_Type matchCfg;

    GPIO_ConfigPin(TRIG_FRONT_PORT, TRIG_FRONT_PIN, 1);
    GPIO_ClearValue(TRIG_FRONT_PORT, (1 << TRIG_FRONT_PIN));

    GPIO_ConfigPin(TRIG_REAR_PORT, TRIG_REAR_PIN, 1);
    GPIO_ClearValue(TRIG_REAR_PORT, (1 << TRIG_REAR_PIN));

    PINSEL_SetPinFunc(ECHO_FRONT_PORT, ECHO_FRONT_PIN_NUM, PINSEL_FUNC_3);
    PINSEL_SetPinFunc(ECHO_REAR_PORT,  ECHO_REAR_PIN_NUM,  PINSEL_FUNC_3);

    matchCfg = TIMER_ConfigStructMatch(TIM_MR0_INT, 60000, DISABLE, DISABLE,
            DISABLE, TIM_EXTMATCH_NOTHING);
    TIMER_Config(LPC_TIM0, TIM_TIMER_MODE, &matchCfg);

    TIMER_ConfigCapture(LPC_TIM0, 0, ENABLE, ENABLE, ENABLE);
    TIMER_ConfigCapture(LPC_TIM0, 1, ENABLE, ENABLE, ENABLE);
}

/*******************************************************************************//**
 * @brief    Inicializa Timer0 y los pines TRIG/ECHO del HC-SR04.
 * @details  Configura los pines TRIG y ECHO de ambos sensores, Timer0 en modo
 * 			 capture para medir los pulsos ECHO y habilita su contador y su
 * 			 interrupción en el NVIC.
 * @note     USW.2.1.7.1 / 2.1.20.1
 **********************************************************************************/
void HCSR04_Init(void)
{
	HCSR04_ConfigHardware();
    TIMER_EnableCounter(LPC_TIM0, ENABLE);
    TIMER_EnableNVIC(TIMER0);
}

/*******************************************************************************//**
 * @brief    Convierte tiempo del pulso ECHO a distancia en centímetros.
 * @details  Se utiliza la fórmula d (cm) = (time_us * 17) / 1000 para mantenernos
 * 			 con numeros enteros.
 * @note     USW.2.1.7.1 / 2.1.20.1
 *
 * @param time_us  Duración del pulso ECHO en microsegundos.
 * @return         Distancia en centímetros.
 **********************************************************************************/
uint32_t HCSR04_GetDistanceCm(uint32_t time_us)
{
    return (time_us * SENSOR_CONST_NUM) / SENSOR_CONST_BACK;
}

/*******************************************************************************//**
 * @brief    Genera los pulsos TRIG de ambos sensores.
 * @details  Se incrementa el contador interno hasta TRIG_PERIOD_MS para saber
 * 			 cuándo enviar los pulsos TRIG a ambos sensores e iniciar la lectura
 * 			 de la distancia. Una vez se envía el pulso, cuando este alcanza su
 * 			 duración se pone en nivel bajo.
 * @note     USW.2.1.7.1 / 2.1.20.1
 **********************************************************************************/
void HCSR04_TriggerUpdate(void)
{
    static uint32_t trigCounter = 0;

    trigCounter++;

    if(trigCounter == TRIG_PERIOD_MS)
    {
        GPIO_SetValue(TRIG_FRONT_PORT, (1 << TRIG_FRONT_PIN));
        GPIO_SetValue(TRIG_REAR_PORT,  (1 << TRIG_REAR_PIN));
    }
    else if(trigCounter >= (TRIG_PERIOD_MS + TRIG_DURATION_MS))
    {
        GPIO_ClearValue(TRIG_FRONT_PORT, (1 << TRIG_FRONT_PIN));
        GPIO_ClearValue(TRIG_REAR_PORT,  (1 << TRIG_REAR_PIN));
        trigCounter = 0;
    }
}
