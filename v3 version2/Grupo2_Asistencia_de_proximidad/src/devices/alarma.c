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
 * @note     ESW - Capa de Dispositivos
 **********************************************************************************/

#include "devices/alarma.h"
#include "lpc17xx_dac.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_pinsel.h"

/* ===== Pines de LEDs RGB (privados) ===== */
#define LED_FRONT_PORT      2
#define LED_FRONT_R_PIN     0
#define LED_FRONT_G_PIN     1
#define LED_REAR_PORT       2
#define LED_REAR_R_PIN      2
#define LED_REAR_G_PIN      3

/* ===== Pines de transistores NPN (privados) ===== */
#define TRANSISTOR_FRONT_PORT   0
#define TRANSISTOR_FRONT_PIN    4
#define TRANSISTOR_REAR_PORT    0
#define TRANSISTOR_REAR_PIN     5

/* ===== Timer1: 1 ms con prescaler 1 µs (privado) ===== */
#define TIMER1_PERIOD_US    1000

/* ===== DAC: COUNT_VALUE para 440 Hz con LUT de 32 muestras (privado) ===== */
/* PCLK_DAC = 25 MHz → T = 1/(440*32) ≈ 71 µs → COUNT = 71e-6 * 25e6 ≈ 1775 */
#define DAC_COUNT_VALUE     1775

/* ===== DMA: macro de control del campo Control de la LLI ===== */
/* Bits 11-0: tamaño; bits 20-18: ancho src (2=32b); bits 23-21: ancho dst (2=32b);
   bit 26: incremento de src; bit 27: sin incremento de dst                         */
#define DMA_CONTROL(size)   ( ((size) & 0xFFF) | (2 << 18) | (2 << 21) | (1 << 26) )

#define DMA_CHANNEL_DAC     0
#define LUT_SIZE            32
#define DAC_CENTER          512

/* ===== Tabla base de la onda senoidal ===== */
static const uint16_t sineBase[LUT_SIZE] =
{
    512, 612, 707, 794, 866, 918, 946, 946,
    918, 866, 794, 707, 612, 512, 412, 317,
    230, 158, 106,  78,  78, 106, 158, 230,
    317, 412, 512, 612, 707, 794, 866, 918
};

/* Buffer de la señal con amplitud ajustada, enviado por DMA al DAC */
static uint32_t currentLUT[LUT_SIZE];

/* LLI del DMA (debe persistir en RAM durante la transferencia) */
static GPDMA_LLI_Type dacLLI;
static uint8_t         dmaReady = 0;

/* ===== Funciones internas de configuración ===== */

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

static void leds_config(void)
{
    gpio_config_output(LED_FRONT_PORT, LED_FRONT_R_PIN);
    gpio_config_output(LED_FRONT_PORT, LED_FRONT_G_PIN);
    gpio_config_output(LED_REAR_PORT,  LED_REAR_R_PIN);
    gpio_config_output(LED_REAR_PORT,  LED_REAR_G_PIN);

    GPIO_ClearValue(LED_FRONT_PORT, (1 << LED_FRONT_R_PIN) | (1 << LED_FRONT_G_PIN));
    GPIO_ClearValue(LED_REAR_PORT,  (1 << LED_REAR_R_PIN)  | (1 << LED_REAR_G_PIN));
}

static void transistores_config(void)
{
    gpio_config_output(TRANSISTOR_FRONT_PORT, TRANSISTOR_FRONT_PIN);
    gpio_config_output(TRANSISTOR_REAR_PORT,  TRANSISTOR_REAR_PIN);

    /* Alto = transistor saturado = buzzer en silencio al inicio */
    GPIO_SetValue(TRANSISTOR_FRONT_PORT, (1 << TRANSISTOR_FRONT_PIN));
    GPIO_SetValue(TRANSISTOR_REAR_PORT,  (1 << TRANSISTOR_REAR_PIN));
}

static void timer1_config(void)
{
    TIM_TIMERCFG_Type cfgTimer;
    TIM_MATCHCFG_Type matchCfg;

    cfgTimer.PrescaleOption = TIM_PRESCALE_USVAL;
    cfgTimer.PrescaleValue  = 1;

    matchCfg.MatchChannel       = TIM_MR0_INT;
    matchCfg.MatchValue         = TIMER1_PERIOD_US - 1;
    matchCfg.IntOnMatch         = ENABLE;
    matchCfg.StopOnMatch        = DISABLE;
    matchCfg.ResetOnMatch       = ENABLE;
    matchCfg.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;

    TIM_Init(LPC_TIM1, TIM_TIMER_MODE, &cfgTimer);
    TIM_ConfigMatch(LPC_TIM1, &matchCfg);
    TIM_Cmd(LPC_TIM1, ENABLE);

    NVIC_SetPriority(TIMER1_IRQn, 2);
    NVIC_EnableIRQ(TIMER1_IRQn);
}

static void dac_config(void)
{
    DAC_CONVERTER_CFG_Type cfgDAC;

    cfgDAC.DBLBUF_ENA = 0;
    cfgDAC.CNT_ENA    = 1;
    cfgDAC.DMA_ENA    = 1;

    /* P0.26 como AOUT (función alternativa 2) */
    gpio_set_alternate(PINSEL_PORT_0, PINSEL_PIN_26, PINSEL_FUNC_2);

    DAC_Init(LPC_DAC);
    DAC_ConfigDAConverterControl(LPC_DAC, &cfgDAC);
    DAC_SetDMATimeOut(LPC_DAC, DAC_COUNT_VALUE);
}

static void dma_config(void)
{
    GPDMA_Channel_CFG_Type dmaCfg;

    GPDMA_Init();

    dacLLI.SrcAddr = (uint32_t)currentLUT;
    dacLLI.DstAddr = (uint32_t)(&LPC_DAC->DACR);
    dacLLI.NextLLI = (uint32_t)(&dacLLI);
    dacLLI.Control = DMA_CONTROL(LUT_SIZE);

    dmaCfg.ChannelNum   = DMA_CHANNEL_DAC;
    dmaCfg.SrcMemAddr   = (uint32_t)currentLUT;
    dmaCfg.DstMemAddr   = (uint32_t)(&LPC_DAC->DACR);
    dmaCfg.TransferSize = LUT_SIZE;
    dmaCfg.TransferWidth= GPDMA_WIDTH_WORD;
    dmaCfg.TransferType = GPDMA_TRANSFERTYPE_M2P;
    dmaCfg.SrcConn      = 0;
    dmaCfg.DstConn      = GPDMA_CONN_DAC;
    dmaCfg.DMALLI       = (uint32_t)(&dacLLI);

    GPDMA_Setup(&dmaCfg);
    LPC_GPDMACH0->DMACCControl = DMA_CONTROL(LUT_SIZE) | (1UL << 31);
    GPDMA_ChannelCmd(DMA_CHANNEL_DAC, ENABLE);

    dmaReady = 1;
}

static void dma_reload(void)
{
    if(!dmaReady) return;

    LPC_GPDMACH0->DMACCSrcAddr  = dacLLI.SrcAddr;
    LPC_GPDMACH0->DMACCDestAddr = dacLLI.DstAddr;
    LPC_GPDMACH0->DMACCLLI      = (uint32_t)&dacLLI;
    LPC_GPDMACH0->DMACCControl  = dacLLI.Control | (1UL << 31);
    GPDMA_ChannelCmd(DMA_CHANNEL_DAC, ENABLE);
}

/* ===== API pública ===== */

/*******************************************************************************//**
 * @brief    Inicializa todo el hardware de la alarma.
 **********************************************************************************/
void Alarma_Init(void)
{
    leds_config();
    transistores_config();
    timer1_config();
    dac_config();
    Alarma_ApplyVolume(0);
    dma_config();
}

/*******************************************************************************//**
 * @brief    Aplica un color al LED RGB del sensor indicado. LEDs de cátodo común.
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
 * @brief    Aplica un estado al transistor NPN del buzzer indicado.
 * @details  PLAY → pin en bajo → transistor en corte → buzzer suena.
 *           MUTE → pin en alto → transistor saturado → silencio.
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
 * @brief    Ajusta la amplitud de la onda senoidal y recarga el DMA.
 * @details  Escala cada muestra de la LUT según el volumen (0-100), la desplaza
 *           << 6 para alinear con el registro DACR, detiene el DMA para evitar
 *           mezcla de muestras y lo recarga desde la LLI.
 **********************************************************************************/
void Alarma_ApplyVolume(uint8_t volume)
{
    int32_t sample;

    GPDMA_ChannelCmd(DMA_CHANNEL_DAC, DISABLE);

    if(volume > 100) volume = 100;

    for(uint8_t i = 0; i < LUT_SIZE; i++)
    {
        sample         = (int32_t)sineBase[i] - DAC_CENTER;
        sample         = DAC_CENTER + (sample * volume) / 100;
        currentLUT[i]  = ((uint32_t)sample << 6);
    }

    dma_reload();
}
