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
 * @note     Driver del sensor ultrasónico HC-SR04, pertenece a la capa de dispositivos.
 **********************************************************************************/

#include "devices/sensor_distancia.h"

/*******************************************************************************//**
 * @brief    Pines GPIO de los pulsos TRIG de ambos sensores.
 * @details  Privados del módulo. El sensor frontal usa P0.0 y el trasero P0.1.
 **********************************************************************************/
#define TRIG_FRONT_PORT      0
#define TRIG_FRONT_PIN       0
#define TRIG_REAR_PORT       0
#define TRIG_REAR_PIN        1

/*******************************************************************************//**
 * @brief    Números de pin de las señales ECHO para la configuración PINSEL.
 * @details  Las máscaras de lectura correspondientes se encuentran en el header,
 *           ya que la ISR del Timer0 las necesita directamente.
 **********************************************************************************/
#define ECHO_FRONT_PIN_NUM   26
#define ECHO_REAR_PIN_NUM    27

/*******************************************************************************//**
 * @brief    Período del pulso TRIG en milisegundos.
 * @details  Cada 60 ms se genera un pulso TRIG de 1 ms para disparar una medición
 *           en ambos sensores.
 **********************************************************************************/
#define TRIG_PERIOD_MS       60

/*******************************************************************************//**
 * @brief    Constantes para el cálculo de distancia a partir del tiempo de ECHO.
 * @details  La fórmula utilizada es: d (cm) = (time_us * 17) / 1000.
 *           Se deriva de la velocidad del sonido (340 m/s), el ida y vuelta del
 *           pulso, y la conversión a centímetros. Se trabaja con enteros para
 *           evitar operaciones en punto flotante.
 **********************************************************************************/
#define SENSOR_CONST_NUM     17
#define SENSOR_CONST_BACK    1000

/*******************************************************************************//**
 * @brief    Configura un pin como salida GPIO con función 0 y modo tristate.
 * @details  Construye el struct de PINSEL con función 0 (GPIO), lo aplica con
 *           PINSEL_ConfigPin() y configura la dirección como salida con
 *           GPIO_SetDir(). Usada para los pines TRIG.
 * @note     Función interna del driver, no expuesta fuera de este módulo.
 *
 * @param port  Puerto del pin (0 a 4).
 * @param pin   Número de pin dentro del puerto (0 a 31).
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
 * @brief    Asigna una función alternativa a un pin sin modificar su dirección.
 * @details  Construye el struct de PINSEL con la función indicada y lo aplica con
 *           PINSEL_ConfigPin(). Usada para habilitar CAP0.0 en P1.26 (función 3)
 *           y CAP0.1 en P1.27 (función 3) para la captura del pulso ECHO.
 * @note     Función interna del driver, no expuesta fuera de este módulo.
 *
 * @param port  Puerto del pin (0 a 4).
 * @param pin   Número de pin dentro del puerto (0 a 31).
 * @param func  Función PINSEL a asignar (0 a 3).
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
 * @details  Inicializa el Timer0 en modo timer con prescaler de 1 µs para que el
 *           contador cuente directamente en microsegundos. Configura un match en
 *           60 000 µs sin interrupción para evitar overflow del contador.
 *           Habilita capture en los canales 0 y 1 por ambos flancos con
 *           interrupción, para detectar el inicio y el fin del pulso ECHO.
 *           El contador queda detenido hasta que SensorDist_Init() lo habilita.
 * @note     Función interna del driver, no expuesta fuera de este módulo.
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
 * @details  Configura los pines TRIG como salidas GPIO y los inicializa en bajo.
 *           Asigna la función alternativa 3 a los pines ECHO para habilitarlos
 *           como CAP0.0 y CAP0.1. Luego configura el Timer0, habilita su contador
 *           e inicializa la interrupción en el NVIC con prioridad 1.
 * @note     Debe llamarse una vez durante la inicialización del sistema, antes de
 *           habilitar interrupciones globales.
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
 * @details  Aplica la fórmula d (cm) = (time_us * 17) / 1000 usando aritmética
 *           entera para evitar operaciones en punto flotante.
 * @note     Se llama desde la ISR del Timer0, por lo que debe ser lo más rápida
 *           posible.
 *
 * @param time_us  Duración del pulso ECHO en microsegundos.
 * @return         Distancia medida en centímetros.
 **********************************************************************************/
uint32_t SensorDist_GetDistanceCm(uint32_t time_us)
{
    return (time_us * SENSOR_CONST_NUM) / SENSOR_CONST_BACK;
}

/*******************************************************************************//**
 * @brief    Genera los pulsos TRIG de ambos sensores de forma periódica.
 * @details  Mantiene TRIG en bajo durante TRIG_PERIOD_MS milisegundos y luego lo
 *           pone en alto durante 1 ms para disparar la medición. El conteo se
 *           reinicia automáticamente al completarse el período.
 * @note     Debe llamarse cada 1 ms desde la ISR del Timer1.
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
