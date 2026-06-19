/*******************************************************************************//**
 * @file     main.c
 * @brief    Programa principal e ISR del sistema de asistencia de proximidad.
 * @details  Inicializa el sistema a través de los servicios y del asistente de
 *           proximidad, luego entra en un bucle infinito procesando las flags de
 *           nueva medición para actualizar la interfaz de usuario.
 *
 *           ISR del Timer0: maneja el capture del pulso ECHO de los dos sensores
 *           HC-SR04. Llama a PA_SetState() con la duración del pulso medido.
 *
 *           ISR del Timer1: llamada cada 1 ms. Ejecuta SC_ShowState() para
 *           controlar los buzzers y DIST_TriggerUpdate() para los pulsos TRIG.
 * @note     ESW - Capa de Aplicación
 **********************************************************************************/

#include "LPC17xx.h"
#include "app/proximity_assistant.h"

/*******************************************************************************//**
 * @brief    Inicializa el sistema y entra en el bucle principal.
 **********************************************************************************/
int main(void)
{
    PA_Init();
    DIST_Init();
    SC_Init();

    while(1)
    {
        if(distFrontFlag)
        {
            distFrontFlag = 0;
            PA_ManageState(SENSOR_FRONT);
        }

        if(distRearFlag)
        {
            distRearFlag = 0;
            PA_ManageState(SENSOR_REAR);
        }
    }

    return 0;
}

/*******************************************************************************//**
 * @brief    ISR del Timer0: capture del pulso ECHO de los sensores HC-SR04.
 * @details  CAP0.0 para el sensor frontal, CAP0.1 para el trasero.
 *           En flanco de subida se guarda el valor capturado como inicio del ECHO.
 *           En flanco de bajada se calcula la duración y se llama a PA_SetState()
 *           para actualizar la distancia, el estado y la flag de nueva medición.
 **********************************************************************************/
void TIMER0_IRQHandler(void)
{
    static uint32_t frontStart = 0;
    static uint32_t rearStart  = 0;
    uint32_t        echoVal;
    uint32_t        echoDuration;

    if(TIM_GetIntStatus(LPC_TIM0, TIM_CR0_INT))
    {
        echoVal = TIM_GetCaptureValue(LPC_TIM0, TIM_COUNTER_INCAP0);

        if(GPIO_ReadValue(ECHO_FRONT_PORT) & ECHO_FRONT_MASK)
        {
            frontStart = echoVal;
        }
        else
        {
            echoDuration = echoVal - frontStart;
            PA_SetState(SENSOR_FRONT, echoDuration);
        }

        TIM_ClearIntPending(LPC_TIM0, TIM_CR0_INT);
    }

    if(TIM_GetIntStatus(LPC_TIM0, TIM_CR1_INT))
    {
        echoVal = TIM_GetCaptureValue(LPC_TIM0, TIM_COUNTER_INCAP1);

        if(GPIO_ReadValue(ECHO_REAR_PORT) & ECHO_REAR_MASK)
        {
            rearStart = echoVal;
        }
        else
        {
            echoDuration = echoVal - rearStart;
            PA_SetState(SENSOR_REAR, echoDuration);
        }

        TIM_ClearIntPending(LPC_TIM0, TIM_CR1_INT);
    }
}

/*******************************************************************************//**
 * @brief    ISR del Timer1: base de tiempo de 1 ms del sistema.
 * @details  Ejecuta SC_ShowState() para la lógica de intercalado de buzzers y
 *           DIST_TriggerUpdate() para los pulsos TRIG de los sensores.
 **********************************************************************************/
void TIMER1_IRQHandler(void)
{
    SC_ShowState();
    DIST_TriggerUpdate();
    TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
}
