#include "ws2812b.h"

/*
 * Arreglo interno de colores.
 *
 * Cada LED necesita tres valores:
 *
 * Rojo
 * Verde
 * Azul
 *
 * Aunque el usuario trabaja en formato RGB, la transmisión hacia la WS2812B
 * se realiza en formato GRB.
 */
static uint8_t ws2812b_red[WS2812B_NUM_LEDS];
static uint8_t ws2812b_green[WS2812B_NUM_LEDS];
static uint8_t ws2812b_blue[WS2812B_NUM_LEDS];


/*
 * Retardo corto basado en instrucciones NOP.
 *
 * La instrucción NOP consume un ciclo de instrucción.
 * En un PIC18, la frecuencia de instrucción es Fosc / 4.
 *
 * Si Fosc = 48 MHz:
 *
 * Finst = 48 MHz / 4 = 12 MHz
 *
 * Cada ciclo de instrucción dura aproximadamente:
 *
 * 1 / 12 MHz = 83.3 ns
 *
 * Los tiempos de la WS2812B son muy pequeños, por lo que estos retardos
 * pueden requerir ajuste dependiendo de la frecuencia real del sistema,
 * el nivel de optimización del compilador y el circuito usado.
 */
static void WS2812B_NopDelay(uint8_t n)
{
    while(n > 0)
    {
        NOP();
        n--;
    }
}


/*
 * Envía un bit lógico hacia la WS2812B.
 *
 * La WS2812B distingue entre 0 y 1 según el tiempo que la línea de datos
 * permanece en estado alto.
 *
 * Bit 0:
 * Alto corto, bajo largo.
 *
 * Bit 1:
 * Alto largo, bajo corto.
 *
 * Esta función está ajustada como punto de partida para Fosc = 48 MHz.
 * Si la tira no responde correctamente, los valores de NOP pueden requerir
 * calibración.
 */
static void WS2812B_SendBit(uint8_t bit_value)
{
    if(bit_value)
    {
        /*
         * Transmisión de un bit lógico 1.
         */
        LATDbits.LATD0 = 1;

        NOP();
        NOP();
        NOP();
        NOP();
        NOP();

        LATDbits.LATD0 = 0;

        NOP();
        NOP();
    }
    else
    {
        /*
         * Transmisión de un bit lógico 0.
         */
        LATDbits.LATD0 = 1;

        NOP();
        NOP();

        LATDbits.LATD0 = 0;

        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
    }
}


/*
 * Envía un byte hacia la WS2812B.
 *
 * Los bits se transmiten desde el bit más significativo hasta el bit
 * menos significativo.
 *
 * Orden de transmisión:
 *
 * bit 7, bit 6, bit 5, bit 4, bit 3, bit 2, bit 1, bit 0.
 */
static void WS2812B_SendByte(uint8_t data)
{
    uint8_t mask;

    for(mask = 0x80; mask > 0; mask >>= 1)
    {
        if(data & mask)
        {
            WS2812B_SendBit(1);
        }
        else
        {
            WS2812B_SendBit(0);
        }
    }
}


/*
 * Inicializa el pin de datos de la tira WS2812B.
 *
 * El pin se configura como salida digital y se deja en estado bajo.
 */
void WS2812B_Init(void)
{
    /*
     * RD0 como pin digital.
     */
    ANSELDbits.ANSELD0 = 0;

    /*
     * RD0 como salida.
     */
    TRISDbits.TRISD0 = 0;

    /*
     * Estado inicial bajo.
     */
    LATDbits.LATD0 = 0;

    /*
     * Apaga el arreglo interno de LEDs.
     */
    WS2812B_Clear();

    /*
     * Envía el estado apagado a la tira.
     */
    WS2812B_Show();
}


/*
 * Guarda el color de un LED específico en memoria.
 *
 * La función verifica que el índice del LED se encuentre dentro del rango
 * permitido antes de guardar los valores.
 */
void WS2812B_SetPixel(uint8_t led, uint8_t red, uint8_t green, uint8_t blue)
{
    if(led >= WS2812B_NUM_LEDS)
    {
        return;
    }

    ws2812b_red[led] = red;
    ws2812b_green[led] = green;
    ws2812b_blue[led] = blue;
}


/*
 * Asigna el mismo color a todos los LEDs de la tira.
 */
void WS2812B_SetAll(uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t i;

    for(i = 0; i < WS2812B_NUM_LEDS; i++)
    {
        WS2812B_SetPixel(i, red, green, blue);
    }
}


/*
 * Apaga todos los LEDs en el arreglo interno.
 */
void WS2812B_Clear(void)
{
    WS2812B_SetAll(0, 0, 0);
}


/*
 * Envía el arreglo interno de colores hacia la tira WS2812B.
 *
 * La WS2812B requiere que los datos se envíen en el orden GRB:
 *
 * Verde
 * Rojo
 * Azul
 *
 * Después de enviar todos los datos, la línea debe permanecer en bajo
 * durante más de 50 us para que los LEDs actualicen su estado.
 */
void WS2812B_Show(void)
{
    uint8_t i;

    /*
     * Se deshabilitan interrupciones para evitar alteraciones en los tiempos
     * de la señal WS2812B.
     */
    INTCON0bits.GIE = 0;

    for(i = 0; i < WS2812B_NUM_LEDS; i++)
    {
        /*
         * Orden de envío requerido por WS2812B:
         * G, R, B.
         */
        WS2812B_SendByte(ws2812b_green[i]);
        WS2812B_SendByte(ws2812b_red[i]);
        WS2812B_SendByte(ws2812b_blue[i]);
    }

    /*
     * Se vuelve a habilitar interrupciones.
     */
    INTCON0bits.GIE = 1;

    /*
     * Pulso de reset.
     *
     * La línea debe permanecer en bajo por más de 50 us.
     */
    WS2812B_LAT = 0;
    __delay_us(80);
}