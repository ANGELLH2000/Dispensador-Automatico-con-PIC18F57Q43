#ifndef LIBKEYPAD_H
#define LIBKEYPAD_H
#include "cabecera.h"


/*
 * Librería para la lectura de un teclado matricial 4x4.
 *
 * La librería utiliza un puerto completo del microcontrolador:
 *
 * Bits 0 a 3:
 * Filas del teclado configuradas como salidas digitales.
 *
 * Bits 4 a 7:
 * Columnas del teclado configuradas como entradas digitales con
 * resistencias pull-up internas.
 *
 * Distribución obligatoria de los pines:
 *
 * Px0 -> Fila 1
 * Px1 -> Fila 2
 * Px2 -> Fila 3
 * Px3 -> Fila 4
 *
 * Px4 -> Columna 1
 * Px5 -> Columna 2
 * Px6 -> Columna 3
 * Px7 -> Columna 4
 *
 * La función Keypad_Read() incluye antirrebote y espera hasta que
 * la tecla sea liberada antes de retornar el carácter detectado.
 */


/*
 * Cantidad de filas y columnas del teclado matricial.
 */
#define KEYPAD_ROWS          4
#define KEYPAD_COLUMNS       4


/*
 * Valor retornado cuando no se detecta ninguna tecla.
 */
#define KEYPAD_NO_KEY        '\0'


/*
 * Tiempo utilizado para reducir el efecto del rebote mecánico.
 *
 * El valor está expresado en milisegundos.
 */
#define KEYPAD_DEBOUNCE_MS   20


/*
 * Estructura de configuración del teclado matricial.
 *
 * Cada teclado utilizado debe tener una variable propia de tipo Keypad.
 *
 * Campos:
 *
 * port:
 * Puntero al registro PORT utilizado para leer las columnas.
 *
 * lat:
 * Puntero al registro LAT utilizado para controlar las filas.
 *
 * tris:
 * Puntero al registro TRIS utilizado para configurar las filas
 * como salidas y las columnas como entradas.
 *
 * ansel:
 * Puntero al registro ANSEL utilizado para configurar todos los
 * pines del teclado como digitales.
 *
 * wpu:
 * Puntero al registro WPU utilizado para activar las resistencias
 * pull-up internas de las columnas.
 */
typedef struct
{
    volatile uint8_t *port;
    volatile uint8_t *lat;
    volatile uint8_t *tris;
    volatile uint8_t *ansel;
    volatile uint8_t *wpu;

} Keypad;


/*
 * Inicializa un teclado matricial 4x4.
 *
 * La función configura:
 *
 * - Los bits 0 a 3 como salidas digitales para las filas.
 * - Los bits 4 a 7 como entradas digitales para las columnas.
 * - Las resistencias pull-up internas de las columnas.
 * - Todas las filas inicialmente en nivel alto.
 *
 * Parámetros:
 *
 * keypad:
 * Dirección de la estructura asociada al teclado.
 *
 * port:
 * Dirección del registro PORT.
 * Ejemplo: &PORTB
 *
 * lat:
 * Dirección del registro LAT.
 * Ejemplo: &LATB
 *
 * tris:
 * Dirección del registro TRIS.
 * Ejemplo: &TRISB
 *
 * ansel:
 * Dirección del registro ANSEL.
 * Ejemplo: &ANSELB
 *
 * wpu:
 * Dirección del registro WPU.
 * Ejemplo: &WPUB
 *
 * Ejemplo:
 *
 * Keypad_Init(&teclado,
 *             &PORTB,
 *             &LATB,
 *             &TRISB,
 *             &ANSELB,
 *             &WPUB);
 */
void Keypad_Init(Keypad *keypad,
                 volatile uint8_t *port,
                 volatile uint8_t *lat,
                 volatile uint8_t *tris,
                 volatile uint8_t *ansel,
                 volatile uint8_t *wpu);


/*
 * Lee una tecla del teclado matricial.
 *
 * La función realiza las siguientes acciones:
 *
 * 1. Recorre las cuatro filas.
 * 2. Lee el estado de las columnas.
 * 3. Detecta la tecla presionada.
 * 4. Aplica un tiempo de antirrebote.
 * 5. Confirma la tecla detectada.
 * 6. Espera hasta que la tecla sea liberada.
 * 7. Retorna el carácter correspondiente.
 *
 * Retorna:
 *
 * '0' a '9':
 * Para las teclas numéricas.
 *
 * 'A' a 'D':
 * Para las teclas de función.
 *
 * '*' o '#':
 * Para las teclas especiales.
 *
 * KEYPAD_NO_KEY:
 * Cuando no existe ninguna tecla presionada.
 */
char Keypad_Read(Keypad *keypad);

#endif
