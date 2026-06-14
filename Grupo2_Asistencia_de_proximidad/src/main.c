/*******************************************************************************//**
 * @file     Grupo2_Asistencia_de_proximidad.c
 * @brief    Programa principal (main) y rutinas de atención a interrupciones.
 * @details  Contiene la función main() que inicializa todos los periféricos y
 *           módulos del sistema, y luego entra en un bucle infinito para procesar
 *           las nuevas mediciones indicadas por las flags.
 *           Además, incluye las implementaciones de las ISR para Timer0 (capture de
 *           los ECHO de los sensores HC-SR04) y Timer1 (base de tiempo del sistema
 *           para los pitidos de los buzzers y pulsos trigger de los sensores).
 * @note     ESW.2.1.1
 **********************************************************************************/

#include "LPC17xx.h"

#include "leds.h"
#include "transistor.h"
#include "buzzer.h"
#include "sound_ctrl.h"
#include "proximity_assistant.h"

/*******************************************************************************//**
 * @brief    Función principal del sistema.
 * @details  Inicializa primero lo relacionado al hardware y luego lo referido a la
 *           lógica del sistema. Entra en un bucle infinito chequeando las flags de
 *           nueva medición de distancia (distFrontFlag y distRearFlag) para
 *           actualizar las salidas de la interfaz de usuario (óptica y acústica)
 *           según corresponda.
 * @note     USW.2.1.1.1
 **********************************************************************************/
int main(void)
{
    HCSR04_Init();
    LED_Init();
    Transistor_Init();
    Buzzer_Init();


    PA_Init();
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
 * @brief    Rutina de Atención de Interrupción (ISR) del Timer0.
 * @details  Maneja los canales de capture: CAP0.0 para el sensor frontal y CAP0.1
 * 			 para el trasero.
 *           En flanco de subida, se guarda el valor capturado del contador como el
 *           inicio del pulso ECHO. En flanco de bajada, se calcula la diferencia de
 *           tiempos para obtener la duración total del pulso ECHO. Este se pasa a
 *           PA_SetState() para actualizar la distancia, el estado lógico y
 *           levantar las flags de nueva medición de distancia (distFrontFlag y
 *           DistRearFlag).
 * @note     USW.2.1.1.1
 **********************************************************************************/
void TIMER0_IRQHandler(void)
{
    static uint32_t frontStart = 0;
    static uint32_t rearStart  = 0;
    uint32_t        echoVal;
    uint32_t        echoDuration;

    if(TIM_GetIntCaptureStatus(LPC_TIM0, TIM_CR0_INT))
    {
        echoVal = TIM_GetCaptureValue(LPC_TIM0, TIM_CR0_INT);

        if(GPIO_ReadValue(ECHO_FRONT_PORT) & ECHO_FRONT_MASK)
        {
            frontStart = echoVal;
        }
        else
        {
            echoDuration = echoVal - frontStart;
            PA_SetState(SENSOR_FRONT, echoDuration);
        }
        TIM_ClearIntCapturePending(LPC_TIM0, TIM_CR0_INT);
    }

    if(TIM_GetIntCaptureStatus(LPC_TIM0, TIM_CR1_INT))
    {
        echoVal = TIM_GetCaptureValue(LPC_TIM0, TIM_CR1_INT);

        if(GPIO_ReadValue(ECHO_REAR_PORT) & ECHO_REAR_MASK)
        {
            rearStart = echoVal;
        }
        else
        {
            echoDuration = echoVal - rearStart;
            PA_SetState(SENSOR_REAR, echoDuration);
        }
        TIM_ClearIntCapturePending(LPC_TIM0, TIM_CR1_INT);
    }
}

/*******************************************************************************//**
 * @brief    Rutina de Atención de Interrupción (ISR) del Timer1.
 * @details  El Timer1 genera una interrupción cada 1 ms, que sirve como base
 *           de tiempo principal del sistema. En cada match de 1 ms se ejecuta
 *           SC_ShowState(), que controla la lógica de intercalado de los buzzers, y
 *           HCSR04_TriggerUpdate(), que genera los pulsos TRIG para los HC-SR04.
 * @note     USW.2.1.1.1
 **********************************************************************************/
void TIMER1_IRQHandler(void)
{
    SC_ShowState();
    HCSR04_TriggerUpdate();
    TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);
}
