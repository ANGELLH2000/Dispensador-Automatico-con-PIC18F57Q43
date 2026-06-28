#ifndef WS2812B_H
#define WS2812B_H

#include "cabecera.h"

/*
 * Librería para control de tira LED WS2812B.
 *
 * Esta librería permite controlar una tira de LEDs direccionables WS2812B
 * mediante una sola línea de datos.
 *
 * La WS2812B requiere una señal digital con tiempos muy precisos. Por esta
 * razón, el pin de salida se configura mediante macros directas y no mediante
 * punteros. El uso de punteros en la transmisión puede agregar instrucciones
 * adicionales y alterar el ancho de los pulsos.
 *
 * Para cambiar el pin utilizado, se deben modificar las definiciones:
 *
 * WS2812B_LAT
 * WS2812B_TRIS
 * WS2812B_ANSEL
 *
 * En esta versión, el pin configurado por defecto es RD0.
 */


/*
 * Cantidad máxima de LEDs que puede manejar la librería.
 *
 * Este valor define el tamaño máximo de los arreglos internos de color.
 *
 * Cada LED requiere 3 bytes de memoria:
 *
 * 1 byte para rojo.
 * 1 byte para verde.
 * 1 byte para azul.
 *
 * Si WS2812B_MAX_LEDS vale 30, la librería reserva:
 *
 * 30 x 3 = 90 bytes de RAM.
 *
 * Este valor no significa que siempre se usarán 30 LEDs. Solo indica
 * el máximo permitido por la librería.
 */
#define WS2812B_MAX_LEDS  30


/*
 * Configuración del pin de datos DIN de la tira WS2812B.
 *
 * En esta configuración se usa el pin RD0 del PIC18F57Q43.
 *
 * Para usar otro pin, se deben cambiar estas tres definiciones.
 *
 * Ejemplo para RD0:
 *
 * #define WS2812B_LAT    LATDbits.LATD0
 * #define WS2812B_TRIS   TRISDbits.TRISD0
 * #define WS2812B_ANSEL  ANSELDbits.ANSELD0
 *
 * Ejemplo para RD1:
 *
 * #define WS2812B_LAT    LATDbits.LATD1
 * #define WS2812B_TRIS   TRISDbits.TRISD1
 * #define WS2812B_ANSEL  ANSELDbits.ANSELD1
 *
 * Ejemplo para RB0:
 *
 * #define WS2812B_LAT    LATBbits.LATB0
 * #define WS2812B_TRIS   TRISBbits.TRISB0
 * #define WS2812B_ANSEL  ANSELBbits.ANSELB0
 */
#define WS2812B_LAT    LATCbits.LATC0
#define WS2812B_TRIS   TRISCbits.TRISC0
#define WS2812B_ANSEL  ANSELCbits.ANSELC0


/*
 * Macros para controlar el estado lógico del pin de datos.
 *
 * WS2812B_HIGH():
 * Coloca la línea DIN en estado alto.
 *
 * WS2812B_LOW():
 * Coloca la línea DIN en estado bajo.
 *
 * Se usan macros porque la WS2812B requiere tiempos muy precisos.
 * Una función normal o una escritura mediante punteros puede agregar
 * instrucciones adicionales y alterar el ancho de los pulsos.
 */
#define WS2812B_HIGH()  WS2812B_LAT = 1
#define WS2812B_LOW()   WS2812B_LAT = 0


/*
 * Estructura de datos para una tira LED WS2812B.
 *
 * Esta estructura almacena la información de color de una tira WS2812B.
 *
 * La estructura no guarda punteros a LAT, TRIS o ANSEL, porque el pin físico
 * de transmisión se define mediante macros para mantener tiempos estables.
 *
 * Campos:
 *
 * num_leds:
 * Cantidad real de LEDs conectados a la tira.
 *
 * red:
 * Arreglo que almacena el valor rojo de cada LED.
 *
 * green:
 * Arreglo que almacena el valor verde de cada LED.
 *
 * blue:
 * Arreglo que almacena el valor azul de cada LED.
 *
 * Nota:
 * Aunque los colores se guardan como RGB, la WS2812B requiere que los datos
 * se transmitan en orden GRB: verde, rojo y azul.
 */
typedef struct
{
    /*
     * Cantidad real de LEDs conectados a la tira.
     */
    uint8_t num_leds;

    /*
     * Valores de intensidad del color rojo para cada LED.
     *
     * Rango por LED: 0 a 255.
     */
    uint8_t red[WS2812B_MAX_LEDS];

    /*
     * Valores de intensidad del color verde para cada LED.
     *
     * Rango por LED: 0 a 255.
     */
    uint8_t green[WS2812B_MAX_LEDS];

    /*
     * Valores de intensidad del color azul para cada LED.
     *
     * Rango por LED: 0 a 255.
     */
    uint8_t blue[WS2812B_MAX_LEDS];

} LED_WS2812B;


/*
 * Inicializa una tira LED WS2812B.
 *
 * Esta función configura el pin DIN como salida digital, coloca la línea
 * de datos en estado bajo e inicializa los arreglos internos de color.
 *
 * Parámetros:
 *
 * tira:
 * Dirección de la estructura LED_WS2812B asociada a la tira.
 *
 * num_leds:
 * Cantidad real de LEDs conectados a la tira.
 *
 * Si num_leds es mayor que WS2812B_MAX_LEDS, la librería limitará la cantidad
 * de LEDs al máximo permitido.
 *
 * Ejemplo:
 *
 * LED_WS2812B tira1;
 * WS2812B_Init(&tira1, 3);
 */
void WS2812B_Init(LED_WS2812B *tira, uint8_t num_leds);


/*
 * Asigna un color RGB a un LED específico de la tira.
 *
 * Esta función no envía la información a la tira física. Solo guarda los
 * valores de color en la estructura.
 *
 * Para que el cambio se vea en la tira, se debe llamar posteriormente a:
 *
 * WS2812B_Show();
 *
 * Parámetros:
 *
 * tira:
 * Dirección de la estructura LED_WS2812B asociada a la tira.
 *
 * led:
 * Índice del LED que se desea modificar.
 *
 * El primer LED tiene índice 0.
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
                      uint8_t blue);


/*
 * Asigna el mismo color RGB a todos los LEDs de la tira.
 *
 * Esta función no transmite datos hacia la tira WS2812B. Solo actualiza los
 * arreglos internos de color.
 *
 * Para que el cambio se vea físicamente, se debe llamar posteriormente a:
 *
 * WS2812B_Show();
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
                    uint8_t blue);


/*
 * Envía hacia la tira WS2812B los colores almacenados en memoria.
 *
 * Esta función recorre todos los LEDs configurados y transmite sus valores
 * de color mediante la línea de datos DIN.
 *
 * La WS2812B requiere que cada LED reciba 24 bits:
 *
 * 8 bits para verde.
 * 8 bits para rojo.
 * 8 bits para azul.
 *
 * Aunque los colores se almacenan como RGB, el orden de transmisión debe
 * ser GRB.
 *
 * Parámetros:
 *
 * tira:
 * Dirección de la estructura LED_WS2812B asociada a la tira.
 */
void WS2812B_Show(LED_WS2812B *tira);


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
void WS2812B_Clear(LED_WS2812B *tira);


/*
 * Enciende todos los LEDs de la tira con un color RGB personalizado.
 *
 * Esta función asigna el mismo color a todos los LEDs y envía el cambio a
 * la tira física.
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
 *
 * Ejemplo:
 *
 * WS2812B_RGB(&tira1, 10, 0, 0);
 *
 * Enciende la tira en rojo con brillo bajo.
 */
void WS2812B_RGB(LED_WS2812B *tira,
                 uint8_t red,
                 uint8_t green,
                 uint8_t blue);

#endif