
#include "LPC17xx.h"
#include "hcsr04.h"
#include "buzzer.h"
#include "timer.h"
#include "gpio.h"

uint32_t			distFront = 100;
uint32_t			distRear = 100;
uint8_t				distFrontFlag = 0;
uint8_t				distRearFlag = 0;
sensor_state 		sensorFrontState = SAFE_ZONE;
sensor_state 		sensorRearState = SAFE_ZONE;


int main(void) {
	HCSR04_Init();
	Buzzer_Init();

    while(1) {

    	if(distFrontFlag)
    	{
    		distFrontFlag = 0;
			switch(sensorFrontState)
			{
			case SAFE_ZONE:

				break;

			case WARNING_ZONE:

				break;

			case DANGEROUS_ZONE:

				break;

			default:
				break;
			}
    	}

    	if(distRearFlag)
    		distRearFlag = 0;
    	{

			switch(sensorRearState)
			{
			case SAFE_ZONE:

				break;

			case WARNING_ZONE:

				break;

			case DANGEROUS_ZONE:

				break;

			default:
				break;
			}
    	}

    }
    return 0 ;
}

void TIMER0_IRQHandler(void)
{
	/*Supongamos este es el Timer para capturar la distancia del sensor frontal
	 * Interrumpe tanto en el flanco de subida como el de bajada en CAP0.0
	 */
	static uint32_t frontStart = 0;
	static uint32_t frontEnd = 0;
	static uint32_t frontTime = 0;
	static uint32_t rearStart = 0;
	static uint32_t rearEnd = 0;
	static uint32_t rearTime = 0;

	if(TIM_GetIntCaptureStatus(LPC_TIM0, TIM_CR0_INT)){
		if(GPIO_ReadValue(1) & ECHO_FRONT_PIN)	//Esto podría a lo mejor ser una función más sencilla en gpio.h
		{
			frontStart = TIM_GetCaptureValue(LPC_TIM0, CR0);
		}
		else
		{
			frontEnd = TIM_GetCaptureValue(LPC_TIM0, CR0);
			frontTime = frontStart - frontEnd;
			distFront = HCSR04_GetDistanceCm(frontTime);
			distFrontFlag = 1;
		}
		TIM_ClearIntCapturePending(LPC_TIM0, TIM_CR0_INT);
	}
	if(TIM_GetIntCaptureStatus(LPC_TIM0, TIM_CR1_INT)){
		if(GPIO_ReadValue(1) & ECHO_REAR_PIN)	//Esto podría a lo mejor ser una función más sencilla en gpio.h
		{
			rearStart = TIM_GetCaptureValue(LPC_TIM0, CR1);
		}
		else
		{
			rearEnd = TIM_GetCaptureValue(LPC_TIM0, CR1);
			rearTime = rearStart - rearEnd;
			distRear = HCSR04_GetDistanceCm(rearTime);
			distRearFlag = 1;
		}
		TIM_ClearIntCapturePending(LPC_TIM0, TIM_CR1_INT);
	}
}

void TIMER1_IRQHandler(void)
{
	/*Supongamos este es el Timer para actualizar los pitidos de los buzzer
	 * Actualiza los contadores internos de cada uno
	 */
	Buzzer_CounterIncrement();
	TIM_ClearIntPending(LPC_TIM1, TIM_MR0_INT);

}

