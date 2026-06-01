
#include "LPC17xx.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_exti.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_dac.h"

/* Definiciones del tablero */
#define FILAS 		4
#define COLUMNAS 	16
#define CELDAS 		(FILAS*COLUMNAS)

/* Definiciones de botones */
#define boton_arriba 	0
#define boton_abajo 	1
#define boton_izq		2
#define boton_der		3
#define boton_dif		4

/* Definiciones de la dificultad en ms */
#define speed_facil		600
#define speed_normal 	350
#define speed_dificil	150

/* Definiciones de los carácteres para la LCD */


/* Estados del juego */
typedef enum
{
	MENU,
	JUGANDO,
	VICTORIA,
	DERROTA
}EstadoJuego;

/* Direcciones del movimiento de la serpiente */
typedef enum
{
	ARRIBA,
	ABAJO,
	IZQUIERDA,
	DERECHA
}Direccion;

/* Dificultades del juego */
typedef enum
{
	FACIL 	= 0,
	NORMAL 	= 1,
	DIFICIL	= 2,
}Dificultad;

/* Cuerpo de la serpiente */
#define largo_maximo CELDAS

typedef struct
{
	uint8_t 	filas[largo_maximo];
	uint8_t 	columnas[largo_maximo];
	uint16_t 	largo;
}Snake;

/* Prototipo de funciones de configuración */
void cfgPins(void);
void cfgTimer(void);
void cfgGPDMA(void);
void cfgGPIOInt(void);
void cfgDAC(void);

/* Prototipo de funciones del juego */


/* Variables globales */
static volatile EstadoJuego g_estado      	= MENU;		//g -> game
static volatile Dificultad 	g_dificultad 	= FACIL;
static volatile Direccion  	g_dir        	= DERECHA;
static volatile Direccion  	g_sig_dir   	= DERECHA;
static Snake       			g_snake;
static uint8_t             	g_manz_fila;
static uint8_t             	g_manz_col;
static volatile uint32_t   	g_tick_ms    	= 0;   		//contador
static volatile uint8_t   	g_last_move  	= 0;   		//timestamp del último movimiento

/* Flags de los botones */
static volatile uint8_t g_arriba	= 0;
static volatile uint8_t g_abajo  	= 0;
static volatile uint8_t g_izq	 	= 0;
static volatile uint8_t	g_der	 	= 0;
static volatile uint8_t g_any_dir	= 0;	//Sirve para salir del Menú si se presiona cualquier dirección (no dificultad)
static volatile uint8_t g_dif		= 0;

/* Definición del antirebote para todos los botones */
#define antirebote_ms 20
static volatile uint32_t antirebote;
static volatile uint32_t antirebote_arriba = 0;
static volatile uint32_t antirebote_abajo = 0;
static volatile uint32_t antirebote_izq = 0;
static volatile uint32_t antirebote_der = 0;
static volatile uint32_t antirebote_dif = 0;

int main(void) {

    while(1)
    {
    	switch(g_estado)
    	{
    		case MENU:

    			break;

    		case JUGANDO:

    			break;

    		case VICTORIA:

    			break;

    		case DERROTA:

    			break;

    		default:

    			break;
    	}

    }

    return 0 ;
}

void EINT3_IRQHandler(void)
{
	uint32_t status = LPC_GPIOINT->IO0IntStatR;

	if(status & (1 << boton_arriba))
	{
		if( (antirebote - antirebote_arriba) >= antirebote_ms )
		{
			g_arriba 	= 1;
			g_any_dir 	= 1:
		}
	}
	if(status & (1 << boton_abajo))
	{
		if( (antirebote - antirebote_arriba) >= antirebote_ms )
		{
			g_abajo 	= 1;
			g_any_dir 	= 1:
		}
	}
	if(status & (1 << boton_izq))
	{
		if( (antirebote - antirebote_arriba) >= antirebote_ms )
		{
			g_izq 		= 1;
			g_any_dir 	= 1:
		}
	}
	if(status & (1 << boton_der))
	{
		if( (antirebote - antirebote_arriba) >= antirebote_ms )
		{
			g_der 		= 1;
			g_any_dir 	= 1:
		}
	}
	if(status & (1 << boton_dif))
	{
		if( (antirebote - antirebote_arriba) >= antirebote_ms )
		{
			g_dif 		= 1;
			g_any_dir 	= 1:
		}
	}
	GPIO_ClearInt(0, LPC_GPIOINT->IO0IntStatR);
}

void TIMER0_IRQHandler(void)
{
	TIM_ClearIntPending(LPC_TIM0, 0);
	antirebote ++;

}



void cfgPins(void)
{
	PINSEL_CFG_Type cfg_pines;
	 uint8_t pines[] = {boton_arriba, boton_abajo,
	                    boton_izq	, boton_der,
	                    boton_dif};
	 	 //Configura los 5 pines 0.0-0.5 como GPIO y entrada. Luego, configura sus interrupciones
	    for (int i = 0; i < 5; i++)
	    {
	        cfg_pines.Portnum   = PINSEL_PORT_0;
	        cfg_pines.Pinnum    = pines[i];
	        cfg_pines.Funcnum   = PINSEL_FUNC_0;
	        cfg_pines.Pinmode   = PINSEL_PINMODE_PULLDOWN;	//para interrupciones por flanco ascendente
	        cfg_pines.OpenDrain = PINSEL_PINMODE_NORMAL;
	        PINSEL_ConfigPin(&cfg_pines);
	        GPIO_SetDir(0, (1 << pines[i]), 0);

	        //Configuración de las interrupciones
	        GPIO_IntCmd(0, (1 << pines[i]), 0);
	        GPIO_ClearInt(0, (1 << pines[i]) );
	    }
        NVIC_SetPriotity(EINT3_IRQn, 1);			//Luego hay que ver bien qué prioridad le damos
        NVIC_EnableIRQ(EINT3_IRQn);
        return;
}

void cfgTimer(void)
{
	TIM_TIMERCFG_Type cfgTimer0;
	TIM_MATCHCFG_Type cfgMatch0Timer0;

	cfgTimer0.PrescaleOption 	= TIM_PRESCALE_USVAL;
	cfgTimer0.PrescalerValue 	= 1;			//Incremento de TC cada 1 us
	TIM_Init(LPC_TIM0, TIM_TIMER_MODE, &cfgTimer0 );

	cfgMatch0Timer0.MatchChannel 		= 0;
	cfgMatch0Timer0.IntOnMatch 			= ENABLE;
	cfgMatch0Timer0.ResetOnMatch 		= ENABLE;
	cfgMatch0Timer0.StopOnMatch			= DISABLE;
	cfgMatch0Timer0.ExtMatchOutputType 	= TIM_EXTMATCH_NOTHING;
	cfgMatch0Timer0.MatchValue			= 1000;	//Match cada 1ms;
	TIM_ConfigMatch(LPC_TIM0, &cfgMatch0Timer0);
	TIM_ClearIntPending(LPC_TIM0, 0);
	NVIC_SetPriotity(TIM0_IRQn, 2);				//Luego hay que ver bien la prioridad que le damos
	NVIC_EnableIRQ(TIM0_IRQn);
	TIM_Cmd(LPC_TIM0, ENABLE);
	return;
}

void cfgGPDMA(void)
{

}

void cfgDAC(void)
{

}
