/*******************************************************************************//**
 * @file     alarma.c
 * @brief    Implementación del driver de la alarma (LEDs, transistores, DAC, DMA).
 * @details  Configura internamente todo el hardware de salida del sistema:
 *
 *           GPIO interno:
 *             - P2.0 y P2.1 (LED frontal R/G), P2.2 y P2.3 (LED trasero R/G)
 *               como salidas GPIO, inicializadas en bajo (LEDs apagados).
 *             - P0.4 y P0.5 como salidas GPIO (bases de transistores NPN),
 *               inicializadas en alto (transistores saturados = buzzers en silencio).
 *
 *           Timer1 interno:
 *             - Prescaler de 1 µs, match en 1 000 µs (1 ms), interrupción
 *               habilitada con reset automático. NVIC prioridad 2.
 *
 *           DAC interno:
 *             - P0.26 como AOUT (función 2). Timeout configurado para 440 Hz
 *               con una LUT de 32 muestras (COUNT_VALUE = 1775 ciclos de PCLK_DAC).
 *             - Solicitudes DMA habilitadas.
 *
 *           DMA interno:
 *             - Canal 0 en modo M2P, LLI circular apuntando a sí misma para
 *               reproducción continua de la onda senoidal hacia DACR.
 *
 *           Ninguna de estas configuraciones se expone fuera de este archivo.
 * @note     Driver de la alarma acústica y óptica, pertenece a la capa de dispositivos.
 **********************************************************************************/

#include "devices/alarma.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"

/*******************************************************************************//**
 * @brief    Pines GPIO de los LEDs RGB frontal y trasero.
 * @details  LED frontal: rojo en P2.0, verde en P2.1.
 *           LED trasero: rojo en P2.2, verde en P2.3.
 *           LEDs de cátodo común: pin en alto enciende el color.
 **********************************************************************************/
#define LED_FRONT_PORT      2
#define LED_FRONT_R_PIN     0
#define LED_FRONT_G_PIN     1
#define LED_REAR_PORT       2
#define LED_REAR_R_PIN      2
#define LED_REAR_G_PIN      3

/*******************************************************************************//**
 * @brief    Pines GPIO de las bases de los transistores NPN de los buzzers.
 * @details  Buzzer frontal en P0.4, buzzer trasero en P0.5.
 *           Pin en alto satura el transistor (silencio); pin en bajo lo corta (sonido).
 **********************************************************************************/
#define TRANSISTOR_FRONT_PORT   0
#define TRANSISTOR_FRONT_PIN    4
#define TRANSISTOR_REAR_PORT    0
#define TRANSISTOR_REAR_PIN     5

/*******************************************************************************//**
 * @brief    Período del match del Timer1 en microsegundos.
 * @details  Con prescaler de 1 µs, un match en 1 000 genera una interrupción
 *           cada 1 ms, que es la base de tiempo principal del sistema.
 **********************************************************************************/
#define TIMER1_PERIOD_US    1000

/*******************************************************************************//**
 * @brief    Valor de timeout del contador del DAC para generar 440 Hz.
 * @details  El DAC trabaja con PCLK_DAC = CCLK/4 = 25 MHz.
 *           Con una LUT de 32 muestras para reproducir un tono de 440 Hz:
 *           frecuencia de muestreo = 440 * 32 = 14 080 Hz → T ≈ 71 µs.
 *           COUNT_VALUE = 71 µs * 25 MHz ≈ 1775 ciclos.
 **********************************************************************************/
#define DAC_COUNT_VALUE     1775

/*******************************************************************************//**
 * @brief    Macro que construye el campo Control de una LLI del DMA.
 * @details  Configura los campos de la siguiente manera:
 *           Bits 11-0  : tamaño de la transferencia.
 *           Bits 20-18 : ancho de fuente = 2 (palabra de 32 bits).
 *           Bits 23-21 : ancho de destino = 2 (palabra de 32 bits).
 *           Bit  26    : incremento de la dirección fuente habilitado.
 *           Bit  27    : incremento de la dirección destino deshabilitado
 *                        (siempre apunta al registro DACR).
 *
 * @param size  Cantidad de palabras a transferir.
 **********************************************************************************/
#define DMA_CONTROL(size)   ( ((size) & 0xFFF) | (2 << 18) | (2 << 21) | (1 << 26) )

/*******************************************************************************//**
 * @brief    Canal del GPDMA asignado a la transferencia hacia el DAC.
 **********************************************************************************/
#define DMA_CHANNEL_DAC     0

/*******************************************************************************//**
 * @brief    Cantidad de muestras de la LUT de la onda senoidal.
 **********************************************************************************/
#define LUT_SIZE            32

/*******************************************************************************//**
 * @brief    Valor central del DAC (mitad de su resolución de 10 bits).
 * @details  Usado como referencia para escalar la amplitud de la señal.
 **********************************************************************************/
#define DAC_CENTER          512

/*******************************************************************************//**
 * @brief    Tabla base de la onda senoidal de 32 muestras centrada en 512.
 * @details  Representa un período completo de la señal senoidal. Alarma_ApplyVolume()
 *           escala su amplitud en función del volumen antes de enviársela al DMA.
 **********************************************************************************/
static const uint16_t sineBase[LUT_SIZE] =
{
    512, 612, 707, 794, 866, 918, 946, 946,
    918, 866, 794, 707, 612, 512, 412, 317,
    230, 158, 106,  78,  78, 106, 158, 230,
    317, 412, 512, 612, 707, 794, 866, 918
};

/*******************************************************************************//**
 * @brief    Buffer de trabajo con la señal escalada que el DMA envía al DAC.
 * @details  Cada muestra está desplazada << 6 para alinearla con los bits del
 *           registro DACR. Se recalcula en cada llamada a Alarma_ApplyVolume().
 **********************************************************************************/
static uint32_t currentLUT[LUT_SIZE];

/*******************************************************************************//**
 * @brief    Descriptor de la LLI circular del DMA para la reproducción continua.
 * @details  Apunta a sí misma en NextLLI para formar un bucle circular, de modo
 *           que el DMA reproduce la señal indefinidamente sin intervención de la CPU.
 *           Debe residir en RAM durante toda la vida de la transferencia.
 **********************************************************************************/
static GPDMA_LLI_Type dacLLI;

/*******************************************************************************//**
 * @brief    Bandera que indica si el DMA fue inicializado correctamente.
 * @details  Se pone en 1 al finalizar dma_config(). dma_reload() la verifica
 *           antes de operar para evitar acceder al canal antes de configurarlo.
 **********************************************************************************/
static uint8_t dmaReady = 0;

/*******************************************************************************//**
 * @brief    Configura un pin como salida GPIO con función 0 y modo tristate.
 * @details  Construye el struct de PINSEL con función 0 (GPIO), lo aplica con
 *           PINSEL_ConfigPin() y configura la dirección como salida con
 *           GPIO_SetDir(). Usada para los pines de LEDs y transistores.
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
 *           PINSEL_ConfigPin(). Usada para habilitar P0.26 como AOUT (función 2)
 *           para la salida del DAC.
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
 * @brief    Configura los pines de los LEDs RGB como salidas e inicializa en bajo.
 * @details  Configura P2.0 y P2.1 (LED frontal) y P2.2 y P2.3 (LED trasero) como
 *           salidas GPIO y los pone en nivel bajo para que los LEDs comiencen apagados.
 * @note     Función interna del driver, no expuesta fuera de este módulo.
 **********************************************************************************/
static void leds_config(void)
{
    gpio_config_output(LED_FRONT_PORT, LED_FRONT_R_PIN);
    gpio_config_output(LED_FRONT_PORT, LED_FRONT_G_PIN);
    gpio_config_output(LED_REAR_PORT,  LED_REAR_R_PIN);
    gpio_config_output(LED_REAR_PORT,  LED_REAR_G_PIN);

    GPIO_ClearValue(LED_FRONT_PORT, (1 << LED_FRONT_R_PIN) | (1 << LED_FRONT_G_PIN));
    GPIO_ClearValue(LED_REAR_PORT,  (1 << LED_REAR_R_PIN)  | (1 << LED_REAR_G_PIN));
}

/*******************************************************************************//**
 * @brief    Configura los pines de los transistores como salidas e inicializa en alto.
 * @details  Configura P0.4 y P0.5 como salidas GPIO y los pone en nivel alto para
 *           saturar los transistores NPN y mantener los buzzers en silencio al inicio.
 * @note     Función interna del driver, no expuesta fuera de este módulo.
 **********************************************************************************/
static void transistores_config(void)
{
    gpio_config_output(TRANSISTOR_FRONT_PORT, TRANSISTOR_FRONT_PIN);
    gpio_config_output(TRANSISTOR_REAR_PORT,  TRANSISTOR_REAR_PIN);

    /* Pin en alto satura el transistor NPN → el buzzer comienza en silencio */
    GPIO_SetValue(TRANSISTOR_FRONT_PORT, (1 << TRANSISTOR_FRONT_PIN));
    GPIO_SetValue(TRANSISTOR_REAR_PORT,  (1 << TRANSISTOR_REAR_PIN));
}

/*******************************************************************************//**
 * @brief    Configura el Timer1 con match de 1 ms e interrupción habilitada.
 * @details  Inicializa el Timer1 con prescaler de 1 µs y un match en 1 000 µs que
 *           genera una interrupción y resetea el contador automáticamente.
 *           Habilita el contador y registra la interrupción en el NVIC con
 *           prioridad 2.
 * @note     Función interna del driver, no expuesta fuera de este módulo.
 **********************************************************************************/
static void timer1_config(void)
{
    TIM_TIMERCFG_Type cfgTimer;
    TIM_MATCHCFG_Type matchCfg;

    /* Prescaler en microsegundos: el contador avanza 1 unidad por cada µs */
    cfgTimer.PrescaleOption = TIM_PRESCALE_USVAL;
    cfgTimer.PrescaleValue  = 1;

    matchCfg.MatchChannel       = TIM_MR0_INT;
    matchCfg.MatchValue         = TIMER1_PERIOD_US - 1; /* match a los 999 µs → período de 1 ms exacto */
    matchCfg.IntOnMatch         = ENABLE;               /* genera interrupción en cada match */
    matchCfg.StopOnMatch        = DISABLE;              /* el contador no se detiene */
    matchCfg.ResetOnMatch       = ENABLE;               /* se reinicia solo para el próximo ciclo */
    matchCfg.ExtMatchOutputType = TIM_EXTMATCH_NOTHING; /* no afecta pines externos */

    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &cfgTimer);
    TIM_ConfigMatch(LPC_TIM1, &matchCfg);
    TIM_Cmd(LPC_TIM1, ENABLE);

    /* Prioridad 2: menor que la del Timer0 (prioridad 1) para no bloquear los captures */
    NVIC_SetPriority(TIMER1_IRQn, 2);
    NVIC_EnableIRQ(TIMER1_IRQn);
}

/*******************************************************************************//**
 * @brief    Inicializa el DAC en P0.26 listo para transferencia continua por DMA.
 * @details  Asigna la función alternativa 2 a P0.26 para usarlo como AOUT.
 *           Inicializa el DAC, habilita el contador interno y las solicitudes DMA,
 *           y carga DAC_COUNT_VALUE en el contador para generar un tono de 440 Hz.
 * @note     Función interna del driver, no expuesta fuera de este módulo.
 **********************************************************************************/
static void dac_config(void)
{
    DAC_CONVERTER_CFG_Type cfgDAC;

    cfgDAC.DBLBUF_ENA = 0; /* doble buffer deshabilitado: no se necesita para DMA continuo */
    cfgDAC.CNT_ENA    = 1; /* habilita el contador interno que cadencia las muestras */
    cfgDAC.DMA_ENA    = 1; /* habilita las solicitudes de DMA en cada timeout del contador */

    /* P0.26 pasa a función 2 (AOUT) para que el DAC tenga salida analógica */
    gpio_set_alternate(PINSEL_PORT_0, PINSEL_PIN_26, PINSEL_FUNC_2);

    DAC_Init(LPC_DAC);
    DAC_ConfigDAConverterControl(LPC_DAC, &cfgDAC);

    /* Carga el timeout del contador: determina la frecuencia de muestreo → 440 Hz */
    DAC_SetDMATimeOut(LPC_DAC, DAC_COUNT_VALUE);
}

/*******************************************************************************//**
 * @brief    Inicializa el GPDMA y configura el canal 0 para enviar la señal al DAC.
 * @details  Inicializa el controlador GPDMA con GPDMA_Init(). Construye la LLI con
 *           fuente en currentLUT, destino en DACR y NextLLI apuntando a sí misma
 *           para formar un bucle circular. Activa el canal 0 con GPDMA_Setup() y
 *           GPDMA_ChannelCmd(). Al finalizar, pone dmaReady en 1.
 * @note     Función interna del driver, no expuesta fuera de este módulo.
 **********************************************************************************/
static void dma_config(void)
{
    GPDMA_Channel_CFG_Type dmaCfg;

    GPDMA_Init();

    /* Construcción de la LLI circular: NextLLI apunta a sí misma para loop infinito */
    dacLLI.SrcAddr = (uint32_t)currentLUT;          /* origen: buffer de la señal en RAM */
    dacLLI.DstAddr = (uint32_t)(&LPC_DAC->DACR);   /* destino: registro de datos del DAC */
    dacLLI.NextLLI = (uint32_t)(&dacLLI);           /* apunta a sí misma → reproducción continua */
    dacLLI.Control = DMA_CONTROL(LUT_SIZE);

    /* Configuración del canal: transferencia de memoria a periférico (M2P) */
    dmaCfg.ChannelNum   = DMA_CHANNEL_DAC;
    dmaCfg.SrcMemAddr   = (uint32_t)currentLUT;
    dmaCfg.DstMemAddr   = (uint32_t)(&LPC_DAC->DACR);
    dmaCfg.TransferSize = LUT_SIZE;
    dmaCfg.TransferWidth= GPDMA_WIDTH_WORD;         /* palabras de 32 bits */
    dmaCfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
    dmaCfg.SrcConn      = 0;                        /* fuente en memoria, sin periférico fuente */
    dmaCfg.DstConn      = GPDMA_CONN_DAC;
    dmaCfg.DMALLI       = (uint32_t)(&dacLLI);      /* encadena la LLI para transferencia circular */

    GPDMA_Setup(&dmaCfg);

    /* Bit 31 del campo Control habilita el bit de terminal count interrupt */
    LPC_GPDMACH0->DMACCControl = DMA_CONTROL(LUT_SIZE) | (1UL << 31);
    GPDMA_ChannelCmd(DMA_CHANNEL_DAC, ENABLE);

    dmaReady = 1;
}

/*******************************************************************************//**
 * @brief    Recarga el canal DMA desde la LLI, reiniciando desde la muestra 0.
 * @details  Recarga los registros SrcAddr, DstAddr, LLI y Control del canal 0
 *           directamente desde dacLLI para que el DMA arranque desde el inicio del
 *           nuevo LUT sin mezclar muestras viejas y nuevas. Solo opera si dmaReady
 *           está en 1, es decir, si dma_config() fue llamada previamente.
 * @note     Función interna del driver, no expuesta fuera de este módulo.
 **********************************************************************************/
static void dma_reload(void)
{
    /* Guarda de seguridad: no operar si el DMA todavía no fue configurado */
    if(!dmaReady) return;

    /* Recarga los registros del canal directamente desde la LLI ya actualizada,
       de modo que el DMA arranque desde la muestra 0 del nuevo buffer */
    LPC_GPDMACH0->DMACCSrcAddr  = dacLLI.SrcAddr;
    LPC_GPDMACH0->DMACCDestAddr = dacLLI.DstAddr;
    LPC_GPDMACH0->DMACCLLI      = (uint32_t)&dacLLI;
    LPC_GPDMACH0->DMACCControl  = dacLLI.Control | (1UL << 31);
    GPDMA_ChannelCmd(DMA_CHANNEL_DAC, ENABLE);
}

/*******************************************************************************//**
 * @brief    Inicializa todo el hardware de la alarma.
 * @details  Ejecuta en orden: configuración de LEDs, transistores, Timer1, DAC,
 *           volumen inicial en 0 y DMA. El volumen se carga antes que el DMA para
 *           que el buffer currentLUT esté listo cuando el canal se active.
 * @note     Debe llamarse una vez durante la inicialización del sistema.
 **********************************************************************************/
void Alarma_Init(void)
{
    leds_config();          /* GPIO de LEDs como salidas, inicializados en bajo */
    transistores_config();  /* GPIO de transistores como salidas, inicializados en alto */
    timer1_config();        /* Timer1: base de tiempo de 1 ms con interrupción */
    dac_config();           /* DAC en P0.26 con timeout para 440 Hz */
    Alarma_ApplyVolume(0);  /* llena currentLUT con volumen 0 antes de arrancar el DMA */
    dma_config();           /* DMA canal 0: LLI circular de currentLUT hacia DACR */
}

/*******************************************************************************//**
 * @brief    Aplica un color al LED RGB del sensor indicado.
 * @details  Selecciona los pines rojo y verde del LED correspondiente al sensor y
 *           los controla para obtener el color pedido. LEDs de cátodo común:
 *           pin en alto enciende el color.
 *           LED_COLOR_OFF    → ambos pines en bajo.
 *           LED_COLOR_GREEN  → solo verde en alto.
 *           LED_COLOR_YELLOW → ambos en alto.
 *           LED_COLOR_RED    → solo rojo en alto.
 * @note     Llamada desde la capa de servicios a través de AC_SetLED().
 *
 * @param id    SENSOR_FRONT o SENSOR_REAR.
 * @param color Color a aplicar (led_color).
 **********************************************************************************/
void Alarma_SetLED(sensor_id id, led_color color)
{
    uint8_t port;
    uint8_t rPin;
    uint8_t gPin;

    if(id == SENSOR_FRONT)
    {
        port = LED_FRONT_PORT;
        rPin = LED_FRONT_R_PIN;
        gPin = LED_FRONT_G_PIN;
    }
    else
    {
        port = LED_REAR_PORT;
        rPin = LED_REAR_R_PIN;
        gPin = LED_REAR_G_PIN;
    }

    switch(color)
    {
    case LED_COLOR_OFF:
        GPIO_ClearValue(port, (1 << rPin) | (1 << gPin));
        break;

    case LED_COLOR_GREEN:
        GPIO_ClearValue(port, (1 << rPin));
        GPIO_SetValue(port, (1 << gPin));
        break;

    case LED_COLOR_YELLOW:
        GPIO_SetValue(port, (1 << rPin) | (1 << gPin));
        break;

    case LED_COLOR_RED:
        GPIO_SetValue(port, (1 << rPin));
        GPIO_ClearValue(port, (1 << gPin));
        break;

    default:
        break;
    }
}

/*******************************************************************************//**
 * @brief    Aplica un estado de encendido o silencio al transistor del buzzer.
 * @details  Controla el pin GPIO correspondiente a la base del transistor NPN:
 *           PLAY → pin en bajo → transistor en corte → el buzzer suena.
 *           MUTE → pin en alto → transistor saturado → el buzzer queda en silencio.
 * @note     Llamada desde la capa de servicios dentro de AC_ShowState().
 *
 * @param id      BUZZER_FRONT o BUZZER_REAR.
 * @param action  PLAY para habilitar sonido, MUTE para silenciar.
 **********************************************************************************/
void Alarma_SetTransistor(buzzer_id id, buzzer_action action)
{
    if(id == BUZZER_FRONT)
    {
        if(action == PLAY)
            GPIO_ClearValue(TRANSISTOR_FRONT_PORT, (1 << TRANSISTOR_FRONT_PIN));
        else
            GPIO_SetValue(TRANSISTOR_FRONT_PORT,   (1 << TRANSISTOR_FRONT_PIN));
    }
    else
    {
        if(action == PLAY)
            GPIO_ClearValue(TRANSISTOR_REAR_PORT, (1 << TRANSISTOR_REAR_PIN));
        else
            GPIO_SetValue(TRANSISTOR_REAR_PORT,   (1 << TRANSISTOR_REAR_PIN));
    }
}

/*******************************************************************************//**
 * @brief    Ajusta la amplitud de la onda senoidal según el volumen indicado.
 * @details  Recalcula las 32 muestras de currentLUT escalando la diferencia de
 *           cada muestra respecto al centro (DAC_CENTER) en función del volumen
 *           (0–100%) y las desplaza << 6 para alinearlas con el registro DACR.
 *           Detiene el canal DMA antes de escribir el buffer para evitar que el
 *           DMA lea muestras viejas y nuevas mezcladas, y lo recarga desde la LLI
 *           al terminar para que arranque desde la muestra 0 del nuevo LUT.
 * @note     Llamada desde la capa de servicios dentro de AC_ShowState().
 *
 * @param volume  Nivel de volumen deseado: 0 para silencio, 100 para máximo.
 **********************************************************************************/
void Alarma_ApplyVolume(uint8_t volume)
{
    int32_t sample;

    /* Detiene el canal para evitar que el DMA lea muestras durante la actualización */
    GPDMA_ChannelCmd(DMA_CHANNEL_DAC, DISABLE);

    if(volume > 100) volume = 100;

    for(uint8_t i = 0; i < LUT_SIZE; i++)
    {
        /* Centra la muestra en cero, escala su amplitud según el volumen y recentra */
        sample        = (int32_t)sineBase[i] - DAC_CENTER;
        sample        = DAC_CENTER + (sample * volume) / 100;

        /* Desplaza << 6 para alinear el valor de 10 bits con los bits del registro DACR */
        currentLUT[i] = ((uint32_t)sample << 6);
    }

    /* Recarga el DMA desde la LLI para que empiece desde la muestra 0 del nuevo LUT */
    dma_reload();
}
