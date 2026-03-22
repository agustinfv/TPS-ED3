#include <lpc17xx.h>
#include <stdio.h>

// Prototipos
void configGPIO(void);
void delay(void);
void resultado(uint8_t jugada_user, uint8_t jugada_LPC);
uint8_t generarJugada(void);
uint8_t mapearJugada(uint32_t botones);

// Variables globales
uint8_t jugada_user = 0;
uint8_t jugada_LPC = 0;

// Matriz de resultados
int matriz_resultado[3][3] = {
    { 0, -1, 1},
    { 1, 0, -1},
    {-1, 1, 0}
};

int main(void) {
    configGPIO();

    while(1) {
        uint32_t boton_presionado;

        // Esperar a que se PRESIONE un botón (valor != 0)
        while((LPC_GPIO0->FIOPIN & 0x07) == 0) {
            delay();
        }

        // En el momento que se presionó, guardo qué botón fue
        boton_presionado = (LPC_GPIO0->FIOPIN) & 0x07; /*
         1 = Piedra (P0.0)
         2 = Papel  (P0.1)
         4 = Tijera (P0.2) */

        // Verificar que sea un valor válido (1, 2 o 4)
        if(boton_presionado == 1 || boton_presionado == 2 || boton_presionado == 4) {

			// Ahora espero a que se SUELTE el botón (vuelva a ser 0)
			while((LPC_GPIO0->FIOPIN & 0x07) != 0) {
				delay();
			}

			// El botón ya fue presionado Y SOLTADO, proceso la jugada
			jugada_user = mapearJugada(boton_presionado);
			jugada_LPC = generarJugada();
			resultado(jugada_user, jugada_LPC);
        } else {
            // Se presionaron múltiples botones o valor inválido
            // Esperar a que se suelte todo y no procesar
            while((LPC_GPIO0->FIOPIN & 0x07) != 0) {
                delay();
            }
            // No llamo a resultado, vuelvo a empezar
        }
    }
}

void configGPIO(void) {
    // GPIO voy a utilizar el puerto 0.
    LPC_PINCON->PINSEL0 &= ~(0x3F3F << 0);  // recordar que no se usa el P0.3
    LPC_PINCON->PINMODE0 |= ( (2 << 0) |    // P0.0 pull-down (10)
                              (2 << 2) |    // P0.1 pull-down (10)
                              (2 << 4) );   // P0.2 pull-down (10)

    // Máscaras
    LPC_GPIO0->FIOMASK &= ~(0b111 << 0);  // Podemos controlar desde el P0.0 al P0.2
    LPC_GPIO0->FIOMASK &= ~(0b111 << 4);  // Podemos controlar desde el P0.4 al P0.6

    // Configurar dirección: P0.0, P0.1, P0.2 como ENTRADAS
    LPC_GPIO0->FIODIR &= ~(0b111 << 0);

    // Configurar dirección: P0.4, P0.5, P0.6 como SALIDAS
    LPC_GPIO0->FIODIR |= (0b111 << 4);

    // Inicializar LEDs apagados
    LPC_GPIO0->FIOCLR = (0b111 << 4);
}

uint8_t mapearJugada(uint32_t botones) {
    int mapa[8] = { -1, 0, 1, -1, 2, -1, -1, -1 };
    return mapa[botones];
}

uint8_t generarJugada(void) {
    // Pendiente implementar
    return 0;
}

void resultado(uint8_t jugada_user, uint8_t jugada_LPC) {
    // Pendiente implementar
}

void delay(void){
	   uint32_t counter;
	   for(counter=0 ; counter<6000000 ; counter++ ){};
}
