#include "ws2812b.h"


/*
 * Inicializa una tira LED WS2812B.
 *
 * Esta función configura el pin DIN como salida digital, coloca la línea de
 * datos en estado bajo e inicializa los arreglos internos de color en cero.
 *
 * Parámetros:
 *
 * tira:
 * Dirección de la estructura LED_WS2812B asociada a la tira.
 *
 * num_leds:
 * Cantidad real de LEDs conectados a la tira.
 *
 * Si num_leds supera el valor definido por WS2812B_MAX_LEDS, la función
 * limita automáticamente la cantidad de LEDs al máximo permitido por la
 * librería.
 */
void WS2812B_Init(LED_WS2812B *tira, uint8_t num_leds)
{
    uint8_t i;

    /*
     * Limita la cantidad de LEDs al máximo permitido por la librería.
     */
    if(num_leds > WS2812B_MAX_LEDS)
    {
        tira->num_leds = WS2812B_MAX_LEDS;
    }
    else
    {
        tira->num_leds = num_leds;
    }

    /*
     * Configura el pin seleccionado como digital.
     *
     * En el registro ANSEL:
     *
     * 0 = pin digital.
     * 1 = pin analógico.
     *
     * La WS2812B necesita una señal digital, por lo tanto el pin DIN debe
     * configurarse como digital.
     */
    WS2812B_ANSEL = 0;

    /*
     * Configura el pin seleccionado como salida.
     *
     * En el registro TRIS:
     *
     * 0 = salida.
     * 1 = entrada.
     *
     * Como el PIC enviará datos hacia la tira, el pin DIN debe estar
     * configurado como salida.
     */
    WS2812B_TRIS = 0;

    /*
     * Coloca la línea de datos en estado bajo.
     *
     * La WS2812B interpreta un nivel bajo mantenido durante más de 50 us
     * como condición de reset o actualización de datos.
     */
    WS2812B_LOW();

    /*
     * Inicializa todos los LEDs como apagados dentro de la memoria.
     *
     * En este punto solo se limpian los arreglos internos. La información
     * todavía no se envía físicamente a la tira.
     */
    for(i = 0; i < tira->num_leds; i++)
    {
        tira->red[i] = 0;
        tira->green[i] = 0;
        tira->blue[i] = 0;
    }
}


/*
 * Asigna un color RGB a un LED específico de la tira.
 *
 * Esta función solo guarda los valores de color en la estructura. No envía
 * datos hacia la tira física. Para que el cambio se vea, posteriormente se
 * debe llamar a WS2812B_Show().
 *
 * Parámetros:
 *
 * tira:
 * Dirección de la estructura LED_WS2812B asociada a la tira.
 *
 * led:
 * Índice del LED que se desea modificar. El primer LED tiene índice 0.
 *
 * red:
 * Intensidad del color rojo. Rango: 0 a 255.
 *
 * green:
 * Intensidad del color verde. Rango: 0 a 255.
 *
 * blue:
 * Intensidad del color azul. Rango: 0 a 255.
 */
void WS2812B_SetPixel(LED_WS2812B *tira,
                      uint8_t led,
                      uint8_t red,
                      uint8_t green,
                      uint8_t blue)
{
    /*
     * Verifica que el LED solicitado exista dentro del rango configurado.
     *
     * Si la tira fue inicializada con 3 LEDs, los índices válidos son:
     *
     * 0, 1 y 2.
     *
     * Si se intenta modificar un LED fuera de rango, la función termina
     * sin realizar cambios.
     */
    if(led >= tira->num_leds)
    {
        return;
    }

    /*
     * Guarda los valores RGB del LED indicado.
     *
     * Aunque internamente se almacenan como RGB, la WS2812B requiere que
     * la transmisión física se haga en orden GRB. Ese cambio de orden se
     * realiza en la función WS2812B_Show().
     */
    tira->red[led] = red;
    tira->green[led] = green;
    tira->blue[led] = blue;
}


/*
 * Asigna el mismo color RGB a todos los LEDs configurados.
 *
 * Esta función recorre la tira desde el LED 0 hasta el último LED configurado
 * y guarda el mismo color en cada posición.
 *
 * La función no envía datos hacia la tira física. Para aplicar el cambio se
 * debe llamar posteriormente a WS2812B_Show().
 *
 * Parámetros:
 *
 * tira:
 * Dirección de la estructura LED_WS2812B asociada a la tira.
 *
 * red:
 * Intensidad del color rojo. Rango: 0 a 255.
 *
 * green:
 * Intensidad del color verde. Rango: 0 a 255.
 *
 * blue:
 * Intensidad del color azul. Rango: 0 a 255.
 */
void WS2812B_SetAll(LED_WS2812B *tira,
                    uint8_t red,
                    uint8_t green,
                    uint8_t blue)
{
    uint8_t i;

    /*
     * Recorre todos los LEDs configurados en la estructura.
     */
    for(i = 0; i < tira->num_leds; i++)
    {
        /*
         * Reutiliza WS2812B_SetPixel() para asignar el color a cada LED.
         */
        WS2812B_SetPixel(tira, i, red, green, blue);
    }
}


/*
 * Envía un bit lógico hacia la tira WS2812B.
 *
 * Esta función es interna de la librería. No debe llamarse desde el programa
 * principal.
 *
 * La WS2812B diferencia un bit 0 de un bit 1 según el tiempo que la línea
 * de datos permanece en estado alto.
 *
 * Bit 0:
 * Pulso alto corto y pulso bajo largo.
 *
 * Bit 1:
 * Pulso alto largo y pulso bajo corto.
 *
 * Esta función utiliza macros directas para modificar el pin de datos:
 *
 * WS2812B_HIGH()
 * WS2812B_LOW()
 *
 * Se evita el uso de punteros o funciones auxiliares en esta parte porque
 * cualquier instrucción adicional puede alterar los tiempos de comunicación.
 *
 * Parámetros:
 *
 * bit_value:
 * Valor lógico a transmitir.
 *
 * Si bit_value es 0, se transmite un bit 0.
 * Si bit_value es diferente de 0, se transmite un bit 1.
 */
static void WS2812B_SendBit(uint8_t bit_value)
{
    if(bit_value)
    {
        /*
         * Transmisión de un bit lógico 1.
         *
         * Para el bit 1, el tiempo en alto debe ser mayor que para el bit 0.
         */
        WS2812B_HIGH();

        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();

        WS2812B_LOW();

        NOP();
        NOP();
    }
    else
    {
        /*
         * Transmisión de un bit lógico 0.
         *
         * Para el bit 0, el tiempo en alto debe ser menor que para el bit 1.
         */
        WS2812B_HIGH();

        NOP();
        NOP();

        WS2812B_LOW();

        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
        NOP();
    }
}


/*
 * Envía un byte hacia la tira WS2812B.
 *
 * Esta función es interna de la librería. No debe llamarse desde el programa
 * principal.
 *
 * Un byte está formado por 8 bits. La WS2812B requiere que los bits se envíen
 * desde el bit más significativo hasta el bit menos significativo.
 *
 * Orden de transmisión:
 *
 * bit 7, bit 6, bit 5, bit 4, bit 3, bit 2, bit 1, bit 0.
 *
 * Parámetros:
 *
 * data:
 * Byte que se desea transmitir.
 */
static void WS2812B_SendByte(uint8_t data)
{
    uint8_t mask;

    /*
     * La máscara inicia en el bit más significativo.
     *
     * 0x80 en binario es:
     *
     * 1000 0000
     *
     * Luego la máscara se desplaza hacia la derecha hasta llegar al bit 0.
     */
    for(mask = 0x80; mask > 0; mask >>= 1)
    {
        /*
         * Evalúa si el bit actual vale 1 o 0.
         *
         * Si data & mask es diferente de cero, el bit evaluado es 1.
         * Si data & mask es cero, el bit evaluado es 0.
         */
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
 * Envía hacia la tira WS2812B los colores almacenados en memoria.
 *
 * Esta función recorre todos los LEDs configurados y transmite sus datos
 * mediante la línea DIN.
 *
 * Cada LED necesita 24 bits:
 *
 * 8 bits para verde.
 * 8 bits para rojo.
 * 8 bits para azul.
 *
 * Aunque los colores se guardan internamente como RGB, la WS2812B requiere
 * recibirlos en orden GRB.
 *
 * Parámetros:
 *
 * tira:
 * Dirección de la estructura LED_WS2812B asociada a la tira.
 */
void WS2812B_Show(LED_WS2812B *tira)
{
    uint8_t i;

    /*
     * Recorre todos los LEDs configurados.
     */
    for(i = 0; i < tira->num_leds; i++)
    {
        /*
         * Orden requerido por la WS2812B:
         *
         * 1. Verde.
         * 2. Rojo.
         * 3. Azul.
         */
        WS2812B_SendByte(tira->green[i]);
        WS2812B_SendByte(tira->red[i]);
        WS2812B_SendByte(tira->blue[i]);
    }

    /*
     * Pulso de reset.
     *
     * Después de enviar todos los datos, la línea debe permanecer en bajo
     * por más de 50 us para que la WS2812B actualice la información recibida.
     *
     * Se utiliza 300 us como margen seguro.
     */
    WS2812B_LOW();
    __delay_us(300);
}


/*
 * Apaga todos los LEDs de la tira.
 *
 * Esta función coloca todos los valores RGB en cero y envía el cambio a la
 * tira física.
 *
 * Parámetros:
 *
 * tira:
 * Dirección de la estructura LED_WS2812B asociada a la tira.
 */
void WS2812B_Clear(LED_WS2812B *tira)
{
    WS2812B_SetAll(tira, 0, 0, 0);
    WS2812B_Show(tira);
}


/*
 * Enciende todos los LEDs con un color RGB personalizado.
 *
 * Esta función asigna el mismo color a todos los LEDs de la tira y envía
 * inmediatamente el cambio a la tira física.
 *
 * Parámetros:
 *
 * tira:
 * Dirección de la estructura LED_WS2812B asociada a la tira.
 *
 * red:
 * Intensidad del color rojo. Rango: 0 a 255.
 *
 * green:
 * Intensidad del color verde. Rango: 0 a 255.
 *
 * blue:
 * Intensidad del color azul. Rango: 0 a 255.
 */
void WS2812B_RGB(LED_WS2812B *tira,
                 uint8_t red,
                 uint8_t green,
                 uint8_t blue)
{
    WS2812B_SetAll(tira, red, green, blue);
    WS2812B_Show(tira);
}