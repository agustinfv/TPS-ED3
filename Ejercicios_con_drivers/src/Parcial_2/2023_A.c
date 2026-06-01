/*
 * Programar el microcontrolador LPC1769 en un código de lenguaje C para que,
 * utilizando un timer y un pin de capture de esta placa sea posible de modular
 *  una señal PWM que ingresa por dicho pin (calcular el ciclo de trabajo y el periodo)  y
 *  sacar una tensión continua proporcional al ciclo de trabajo a través del DAC de rango
 *  dinámico 0-2V con un rate de actualización de 0,5s del promedio de las últimos diez
 *   valores obtenidos en la captura.
 *
 */

#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"

#define MUESTRA 10

static volatile uint32_t array[10];
static volatile uint16_t indice = 0;

void configTIMER();
void configPIN();
void configDAC();

int main(void){
	configPIN();
	configTIMER();
	configDAC();
	while(1){

	}
}
/*
 * Configuro el pin de captura utilizando timer1 canal 0 y tmb configuro el pin de salida del DAC
 */

void configPIN(){
	PINSEL_CFG_Type p1_18;
	PINSEL_CFG_Type AOUT;

	p1_18.Portnum = PINSEL_PORT_1;
	p1_18.Pinnum = PINSEL_PIN_18;
	p1_18.Funcnum = PINSEL_FUNC_3;
	p1_18.Pinmode = PINSEL_PINMODE_TRISTATE;
	p1_18.OpenDrain = PINSEL_PINMODE_NORMAL;

	AOUT.Portnum = PINSEL_PORT_0;
	AOUT.Pinnum = PINSEL_PIN_26;
	AOUT.Funcnum = PINSEL_FUNC_2;
	AOUT.Pinmode = PINSEL_PINMODE_TRISTATE;
	AOUT.OpenDrain = PINSEL_PINMODE_NORMAL;

	PINSEL_ConfigPin(&p1_18);
	PINSEL_ConfigPin(&AOUT);


}


void configDAC(){
	DAC_CONVERTER_CFG_Type cfg;

	cfg.DBLBUF_ENA = 0;
	cfg.CNT_ENA = 0;
	cfg.DMA_ENA = 0;

	DAC_Init(&cfg);

}




void configTIMER(){
	TIM_TIMERCFG_Type timer1_0;
	TIM_CAPTURECFG_Type cap1_0;


	timer1_0.PrescaleOption = TIM_PRESCALE_USVAL;
	timer1_0.PrescaleValue = 1; //cuento cada 1 uSeg
	//countT1_0. CountInputSelect = //nose que colocar aqui ya que la variable no tiene comentarios en su .h

	cap1_0.CaptureChannel = 0;
	cap1_0.RisingEdge = ENABLE;
	cap1_0.FallingEdge = ENABLE;
	cap1_0.IntOnCaption = ENABLE;

	TIM_Init(LPC_TIM1,TIM_TIMER_MODE,&countT1_0);
	TIM_ConfigCapture(LPC_TIM1,&cap1_0);
	TIM_Cmd(LPC_TIM1,ENABLE);
	NVIC_EnableIRQ(TIMER1_IRQn);
	TIM_ResetCounter(LPC_TIM1);

}

void TIMER1_IRQHandler(void){

	if(TIM_GetIntCaptureStatus(LPC_TIM1,TIM_CR0_INT)){

		static volatile uint16_t estado = 0;
		static volatile uint32_t periodo = 0;
		static volatile uint32_t t_HIGH;
		static volatile uint32_t t_LOW;

		static volatile bool flagPeriodo = false;

		//Interrumpio flanco de subida o bajada, voy a preguntar cual de los dos


		if(LPC_GPIO1->FIOPIN & (1<<18) == SET)
		{
			//sucedio flanco de subida
			switch(estado){
			case 0:
				TIM_ResetCounter(LPC_TIM1);
				estado = 1;
				break;
			}
			case 1:
			case 2:
				t_LOW = TIM_GetCaptureValue(LPC_TIM1,TIM_COUNTER_INCAP0);
				TIM_ResetCounter(LPC_TIM1);
				//Calculo el periodo 1 vez cada 10 capturas
				estado = 3;

			case 3:

				if(flagPeriodo == false){
					periodo = t_LOW + t_HIGH;
					flagPeriodo = true;
				}
				estado = 4;
			case 4:

				array[indice] = (t_HIGH / periodo) * 100;//guarde el valor en porcentaje
				indice =(indice+1) % 10;
				t_HIGH = 0;
				t_LOW = 0;
				/*
				if(indice == RESET){
					promedio =0;
					for(volatile int i = 0; i <= MUESTRA;i++){
						promedio += array[i];
					}
					promedio = promedio /MUESTRA;
					estado = 5;
				}
				*/

				break;
			case 5:
				//en este estado debo enviar promedio


		}else{
			//sucedio flanco de bajada

			swtich(estado){
				case 0:
				case 1:
					t_HIGH = TIM_GetCaptureValue(LPC_TIM1,TIM_COUNTER_INCAP0);
					TIM_ResetCounter(LPC_TIM1);
					estado = 2;
					break;
			}


		}


	}
	//Se que aun no configure el timer1 canal 0 en modo match, lo hare despues.
	if(TIM_GetIntStatus(LPC_TIM1,TIM_MR0_INT))
	{
		//se produjo una int a los 0.5seg
		uint32_t promedio = 0;
		for(volatile int i = 0; i <= MUESTRA;i++){
			promedio += array[i];
		}
		promedio = promedio /MUESTRA;
		promedio = ((promedio * (2000 - 0))/1094) + 2000; //2000 = 2V, nose si es correcto 2000 o 2
		DAC_UpdateValue(LPC_DAC,promedio);

	}

	//limpieza de flags
	TIM_ClearIntPending(LPC_TIM1,TIM_MR0_INT);
	TIM_ClearIntCapturePending(LPC_TIM1,TIM_CR0_INT);


}

/*
 * 		switch(LPC_GPIO1->FIOPIN & (1<<28)){
		case 0:
			//interrumpio flanco de bajada
			swtich(estado){
		case 0:
			}

			t_HIGH = TIM_GetCaptureValue(LPC_TIM1,TIM_COUNTER_INCAP0);
			TIM_ResetCounter(LPC_TIM1);

		case 1:
			//interrumpio flanco de subida
			//Voy a calcular el periodo una sola vez
			switch(estado){

			case 0:
				TIM_ResetCounter(LPC_TIM1);
				//flagPeriodo = true;
				estado = 1;
				break;
			case 1:
				//periodo = periodo + TIM_GetCaptureValue(LPC_TIM1,TIM_COUNTER_INCAP0);//periodo en uS
				//TIM_ResetCounter(LPC_TIM1);
				t_LOW = TIM_GetCaptureValue(LPC_TIM1,TIM_COUNTER_INCAP0);
				TIM_ResetCounter(LPC_TIM1);
				//Envio dato del periodo
				if(flagPeriodo == false){
					periodo = t_LOW + t_HIGH;
					flagPeriodo = true;
				}

				array[indice] = (t_HIGH / periodo) * 100;//guarde el valor en porcentaje
				indice =(indice+1) % 10;
				if(indice == RESET){
					uint32_t promedio =0;
					for(volatile int i = 0; i <= 10;i++){
						promedio += array[i];
					}

				}

			}

		}
 */





