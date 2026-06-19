/*******************************************************************************//**
 * @file     sensor_distancia.c
 * @brief    Implementación del driver del sensor ultrasónico HC-SR04.
 * @details  Contiene la configuración completa del hardware necesario para operar
 *           los dos sensores HC-SR04 del sistema:
 *
 *           GPIO interno:
 *             - P0.0 y P0.1 como salidas GPIO para los pulsos TRIG.
 *             - P1.26 y P1.27 como función alternativa 3 (CAP0.0 y CAP0.1).
 *
 *           Timer0 interno:
 *             - Prescaler de 1 µs para medir tiempos directamente en microsegundos.
 *             - Capture en ambos flancos sobre los canales 0 y 1.
 *             - Interrupción habilitada en el NVIC con prioridad 1.
 *
 *           Estas configuraciones no se exponen fuera de este archivo.
 * @note     ESW - Capa de Dispositivos
 **********************************************************************************/

#include "devices/sensor_distancia.h"

/* ---- Pines TRIG (privados, no expuestos en el header) ---- */
#define TRIG_FRONT_PORT      0
#define TRIG_FRONT_PIN       0
#define TRIG_REAR_PORT       0
#define TRIG_REAR_PIN        1

/* ---- Números de pin de los ECHO (para PINSEL; las máscaras van en el header) ---- */
#define ECHO_FRONT_PIN_NUM   26
#define ECHO_REAR_PIN_NUM    27

/* ---- Período del pulso TRIG: cada 60 ms se dispara 1 ms de pulso ---- */
#define TRIG_PERIOD_MS       60

/* ---- Constantes para d(cm) = (time_us * 17) / 1000 ---- */
#define SENSOR_CONST_NUM     17
#define SENSOR_CONST_BACK    1000

/*******************************************************************************//**
 * @brief    Configura un pin como GPIO salida (función 0, modo tristate).
 **********************************************************************************/
static void gpio_config_output(uint8_t port, uint8_t pin)
{
    PINSEL_CFG_Type pinCfg;

    pinCfg.Portnum   = port;
    pinCfg.Pinnum    = pin;
    pinCfg.Funcnum   = PINSEL_FUNC_0;
    pinCfg.Pinmode   = PINSEL_PINMODE_TRISTATE;
    pinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;

    PINSEL_ConfigPin(&pinCfg);
    GPIO_SetDir(port, (1 << pin), 1);
}

/*******************************************************************************//**
 * @brief    Asigna una función alternativa a un pin (sin cambiar dirección).
 * @details  Usado para habilitar CAP0.0 (P1.26, func 3) y CAP0.1 (P1.27, func 3).
 **********************************************************************************/
static void gpio_set_alternate(uint8_t port, uint8_t pin, uint8_t func)
{
    PINSEL_CFG_Type pinCfg;

    pinCfg.Portnum   = port;
    pinCfg.Pinnum    = pin;
    pinCfg.Funcnum   = func;
    pinCfg.Pinmode   = PINSEL_PINMODE_TRISTATE;
    pinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;

    PINSEL_ConfigPin(&pinCfg);
}

/*******************************************************************************//**
 * @brief    Configura el Timer0 con prescaler de 1 µs y capture en ambos canales.
 * @details  Match en 60 000 µs sin interrupción (solo para evitar overflow).
 *           Capture en canales 0 y 1, ambos flancos, interrupción habilitada.
 *           El contador queda detenido hasta que SensorDist_Init() lo habilita.
 **********************************************************************************/
static void timer0_config(void)
{
    TIM_TIMERCFG_Type   cfgTimer;
    TIM_MATCHCFG_Type   matchCfg;
    TIM_CAPTURECFG_Type capCfg;

    cfgTimer.PrescaleOption = TIM_PRESCALE_USVAL;
    cfgTimer.PrescaleValue  = 1;

    matchCfg.MatchChannel       = TIM_MR0_INT;
    matchCfg.MatchValue         = 60000 - 1;
    matchCfg.IntOnMatch         = DISABLE;
    matchCfg.StopOnMatch        = DISABLE;
    matchCfg.ResetOnMatch       = DISABLE;
    matchCfg.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;

    TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimer);
    TIM_ConfigMatch(LPC_TIM0, &matchCfg);
    TIM_Cmd(LPC_TIM0, DISABLE);

    capCfg.RisingEdge   = ENABLE;
    capCfg.FallingEdge  = ENABLE;
    capCfg.IntOnCaption = ENABLE;

    capCfg.CaptureChannel = 0;
    TIM_ConfigCapture(LPC_TIM0, &capCfg);

    capCfg.CaptureChannel = 1;
    TIM_ConfigCapture(LPC_TIM0, &capCfg);

    TIM_ResetCounter(LPC_TIM0);
}

/*******************************************************************************//**
 * @brief    Inicializa los pines TRIG/ECHO y el Timer0 en modo capture.
 **********************************************************************************/
void SensorDist_Init(void)
{
    gpio_config_output(TRIG_FRONT_PORT, TRIG_FRONT_PIN);
    GPIO_ClearValue(TRIG_FRONT_PORT, (1 << TRIG_FRONT_PIN));

    gpio_config_output(TRIG_REAR_PORT, TRIG_REAR_PIN);
    GPIO_ClearValue(TRIG_REAR_PORT, (1 << TRIG_REAR_PIN));

    gpio_set_alternate(ECHO_FRONT_PORT, ECHO_FRONT_PIN_NUM, PINSEL_FUNC_3);
    gpio_set_alternate(ECHO_REAR_PORT,  ECHO_REAR_PIN_NUM,  PINSEL_FUNC_3);

    timer0_config();

    TIM_Cmd(LPC_TIM0, ENABLE);
    NVIC_SetPriority(TIMER0_IRQn, 1);
    NVIC_EnableIRQ(TIMER0_IRQn);
}

/*******************************************************************************//**
 * @brief    Convierte el tiempo del pulso ECHO a distancia en centímetros.
 **********************************************************************************/
uint32_t SensorDist_GetDistanceCm(uint32_t time_us)
{
    return (time_us * SENSOR_CONST_NUM) / SENSOR_CONST_BACK;
}

/*******************************************************************************//**
 * @brief    Genera los pulsos TRIG de ambos sensores.
 * @details  Mantiene TRIG bajo durante TRIG_PERIOD_MS ms y luego lo pone en alto
 *           1 ms. Debe llamarse cada 1 ms desde la ISR del Timer1.
 **********************************************************************************/
void SensorDist_TriggerUpdate(void)
{
    static uint32_t trigCounter = 0;

    trigCounter++;

    if(trigCounter <= TRIG_PERIOD_MS)
    {
        GPIO_ClearValue(TRIG_FRONT_PORT, (1 << TRIG_FRONT_PIN));
        GPIO_ClearValue(TRIG_REAR_PORT,  (1 << TRIG_REAR_PIN));
    }
    else
    {
        GPIO_SetValue(TRIG_FRONT_PORT, (1 << TRIG_FRONT_PIN));
        GPIO_SetValue(TRIG_REAR_PORT,  (1 << TRIG_REAR_PIN));
        trigCounter = 0;
    }
}
