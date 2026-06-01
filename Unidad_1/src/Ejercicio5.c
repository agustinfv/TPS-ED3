#include "LPC17xx.h"

/*
 * Escribir un programa en C que permita realizar un promedio móvil con los
últimos 8 datos ingresados por el puerto 1. Considerar que cada dato es un
entero signado y está formado por los 16 bits menos significativos de dicho
puerto. El resultado, también de 16 bits, debe ser sacado por los pines P0.0 al
P0.11 y P0.15 al P0.18. Recordar que en un promedio móvil primero se
descarta el dato mas viejo de los 8 datos guardados, se ingresa un nuevo dato
proveniente del puerto y se realiza la nueva operación de promedio con esos 8
datos disponibles, así sucesivamente. Considerar el uso de un retardo antes de
tomar una nueva muestra por el puerto
 */

//promedio de los ultimos 8 datos ingresador por el p1, cada dato esta formado por los 16 bits menos significativos del p1.
//Resultado es de 16 bit y debe ser sacado por los pines P0.0 al P0.11 y p0.15 al p0.18
void delay();

int main(void){
	//GPIO
	LPC_PINCON->PINSEL2 &= ~(0xFFFF);// configure todo el puerto1
	LPC_PINCON->PINSEL0 &= ~(0xFFFF);//CONFIGURE del p0.0 al p.15
	LPC_PINCON->PINSEL1 &= ~(0x3F << 0);// cprresponde del p0.16 al p0.18

	//entrada, corresponde a todo el puerto1
	LPC_GPIO1->FIODIRL &= ~(0xFF);// es un entero signado

	//salida, puerto 0
	LPC_GPIO0->FIODIR |= (0xF << 0);
	LPC_GPIO0->FIODIR |= (0x3 << 16);


}
