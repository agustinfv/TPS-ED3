#include "LPC17xx.h"


/*
 * Utilizando interrupciones por GPIO realizar un código en C que permita, mediante 4 pines de entrada GPIO,
 *  leer y guardar un número compuesto por 4 bits. Dicho número puede ser cambiado por un usuario mediante 4 switches,
 *  los cuales cuentan con sus respectivas resistencias de pull up externas.
 *  El almacenamiento debe realizarse en una variable del tipo array de forma tal que se asegure tener disponible siempre
 *   los últimos 10 números elegidos por el usuario, garantizando además que el número ingresado más antiguo, de este conjunto
 *    de 10, se encuentre en el elemento 9 y el número actual en el elemento 0 de dicho array.
 *    La interrupción por GPIO empezará teniendo la máxima prioridad de interrupción posible y cada 200 números ingresados
 *  deberá disminuir en 1 su prioridad hasta alcanzar la mínima posible. Llegado este momento, el programa deshabilitará todo
 *  tipo de interrupciones producidas por las entradas GPIO. Tener en cuenta que el código debe estar debidamente comentado.
 *
 *
 */

void configINTGPIO();//4 pines resistencia interna deshabilitada

volatile uint32_t indice_numeros = 0;
uint8_t array[10];
uint8_t first_array = 1;
uint8_t ind_prioridad= 1;


int main(void){
	configINTGPIO();

	while(1){

	}
}

void configINTGPIO(){
	LPC_PINCON->PINSEL0 &= ~(0xFF<<0);
	LPC_PINCON->PINMODE0 = (LPC_PINCON->PINMODE0 & ~(0xFF<<0)) | (0xAA<<0);//deshabilito resistencia interna
	LPC_GPIO0->FIODIR &= ~(0xF<<0);
	LPC_GPIOINT->IO0IntClr = (0xF<<0);
	LPC_GPIOINT->IO0IntEnF |= (0xF<<0);
	LPC_GPIOINT->IO0IntEnR |= (0xF<<0);
	NVIC_SetPriority(EINT3_IRQn,0);
	NVIC_EnableIRQ(EINT3_IRQn);
}

void EINT3_IRQHandler(void){
	if(LPC_GPIOINT->IntStatus & (1<<0)){
		//se produjo interrupcion
		uint8_t valor = (LPC_GPIO0->FIOPIN & (0xF<<0));//guardo el valor tengo mis dudas con respecto a que tiene pull-up externa
		if(first_array == 1)
		{
			array[0] = valor;
			first_array =0;
		}
		else{

			for(int i = 8;i > -1;i--){

						array[i+1] = array[i];

					}
			array[0] = valor;
		}
		indice_numeros++;
		if(indice_numeros <= 200)
		{


			if(ind_prioridad < 31){
				ind_prioridad++;
				NVIC_SetPriority(EINT3_IRQn,ind_prioridad);

			}
			else{
				NVIC_DisableIRQ(EINT3_IRQn);
			}
			indice_numeros = 0;
		}
		else{

		}

	}
	else{
		//ruido
	}
	LPC_GPIOINT->IO0IntClr = (0xF<<0);
}

