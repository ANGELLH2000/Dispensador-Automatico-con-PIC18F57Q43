#include "Keypad4x4.h"


/*
 * Máscaras correspondientes a las filas.
 *
 * Las filas deben conectarse a los cuatro bits inferiores
 * del puerto seleccionado.
 *
 * Fila 1 -> bit 0
 * Fila 2 -> bit 1
 * Fila 3 -> bit 2
 * Fila 4 -> bit 3
 */
static const uint8_t Keypad_RowMasks[KEYPAD_ROWS] =
{
    0x01,
    0x02,
    0x04,
    0x08
};


/*
 * Máscaras correspondientes a las columnas.
 *
 * Las columnas deben conectarse a los cuatro bits superiores
 * del puerto seleccionado.
 *
 * Columna 1 -> bit 4
 * Columna 2 -> bit 5
 * Columna 3 -> bit 6
 * Columna 4 -> bit 7
 */
static const uint8_t Keypad_ColumnMasks[KEYPAD_COLUMNS] =
{
    0x10,
    0x20,
    0x40,
    0x80
};


/*
 * Máscara conjunta de las cuatro filas.
 *
 * Bits 0 a 3 en nivel lógico 1.
 */
#define KEYPAD_ROW_MASK       0x0F


/*
 * Máscara conjunta de las cuatro columnas.
 *
 * Bits 4 a 7 en nivel lógico 1.
 */
#define KEYPAD_COLUMN_MASK    0xF0


/*
 * Mapa correspondiente a un teclado matricial 4x4 estándar.
 *
 *              COLUMNAS
 *
 *             C1   C2   C3   C4
 *
 * Fila 1      1    2    3    A
 * Fila 2      4    5    6    B
 * Fila 3      7    8    9    C
 * Fila 4      *    0    #    D
 */
static const char Keypad_Map[KEYPAD_ROWS][KEYPAD_COLUMNS] =
{
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};


/*
 * Coloca todas las filas en nivel alto.
 *
 * La operación OR modifica únicamente los cuatro bits inferiores
 * del registro LAT, conservando el estado de los demás bits.
 */
static void Keypad_AllRowsHigh(Keypad *keypad)
{
    *(keypad->lat) =
        (uint8_t)(*(keypad->lat) | KEYPAD_ROW_MASK);
}


/*
 * Activa una fila específica para realizar el barrido.
 *
 * El método de lectura trabaja con lógica activa en bajo:
 *
 * - Todas las filas se mantienen normalmente en alto.
 * - La fila que se desea revisar se coloca en bajo.
 * - Si una tecla está presionada, la columna correspondiente
 *   también pasa a nivel bajo.
 *
 * Parámetros:
 *
 * keypad:
 * Dirección de la estructura asociada al teclado.
 *
 * row:
 * Número de fila que se desea activar.
 * Rango válido: 0 a 3.
 */
static void Keypad_ActivateRow(Keypad *keypad, uint8_t row)
{
    /*
     * Coloca todas las filas en alto.
     */
    Keypad_AllRowsHigh(keypad);

    /*
     * Coloca únicamente la fila seleccionada en nivel bajo.
     */
    *(keypad->lat) =
        (uint8_t)(*(keypad->lat) &
        (uint8_t)(~Keypad_RowMasks[row]));
}


/*
 * Realiza un barrido inmediato del teclado.
 *
 * Esta función es de uso interno. No aplica antirrebote y tampoco
 * espera a que la tecla sea liberada.
 *
 * Retorna:
 *
 * El carácter correspondiente a la tecla detectada.
 *
 * KEYPAD_NO_KEY:
 * Si no existe ninguna tecla presionada.
 */
static char Keypad_Scan(Keypad *keypad)
{
    uint8_t row;
    uint8_t column;

    /*
     * Recorre las cuatro filas del teclado.
     */
    for(row = 0; row < KEYPAD_ROWS; row++)
    {
        /*
         * Coloca la fila actual en nivel bajo.
         */
        Keypad_ActivateRow(keypad, row);

        /*
         * Espera brevemente para permitir que los niveles eléctricos
         * del puerto se estabilicen antes de leer las columnas.
         */
        __delay_us(5);

        /*
         * Recorre las cuatro columnas.
         */
        for(column = 0; column < KEYPAD_COLUMNS; column++)
        {
            /*
             * Las columnas se mantienen normalmente en nivel alto
             * mediante las resistencias pull-up.
             *
             * Si una columna se encuentra en nivel bajo, significa que
             * una tecla conecta esa columna con la fila activa.
             */
            if((*(keypad->port) &
                Keypad_ColumnMasks[column]) == 0)
            {
                /*
                 * Restaura todas las filas en nivel alto antes
                 * de terminar el barrido.
                 */
                Keypad_AllRowsHigh(keypad);

                /*
                 * Retorna el carácter correspondiente a la fila
                 * y columna detectadas.
                 */
                return Keypad_Map[row][column];
            }
        }
    }

    /*
     * Restaura todas las filas después del barrido.
     */
    Keypad_AllRowsHigh(keypad);

    /*
     * No se detectó ninguna tecla.
     */
    return KEYPAD_NO_KEY;
}


/*
 * Inicializa el teclado matricial 4x4.
 */
void Keypad_Init(Keypad *keypad,
                 volatile uint8_t *port,
                 volatile uint8_t *lat,
                 volatile uint8_t *tris,
                 volatile uint8_t *ansel,
                 volatile uint8_t *wpu)
{
    /*
     * Guarda las direcciones de los registros dentro de la estructura.
     */
    keypad->port  = port;
    keypad->lat   = lat;
    keypad->tris  = tris;
    keypad->ansel = ansel;
    keypad->wpu   = wpu;

    /*
     * Configura todos los pines del puerto como digitales.
     *
     * La librería utiliza los ocho bits del puerto:
     *
     * Bits 0 a 3 -> filas.
     * Bits 4 a 7 -> columnas.
     *
     * En ANSEL:
     * 0 = digital.
     * 1 = analógico.
     */
    *(keypad->ansel) =
        (uint8_t)(*(keypad->ansel) &
        (uint8_t)(~(KEYPAD_ROW_MASK |
                    KEYPAD_COLUMN_MASK)));

    /*
     * Configura los cuatro bits inferiores como salidas.
     *
     * En TRIS:
     * 0 = salida.
     * 1 = entrada.
     */
    *(keypad->tris) =
        (uint8_t)(*(keypad->tris) &
        (uint8_t)(~KEYPAD_ROW_MASK));

    /*
     * Configura los cuatro bits superiores como entradas.
     */
    *(keypad->tris) =
        (uint8_t)(*(keypad->tris) |
                  KEYPAD_COLUMN_MASK);

    /*
     * Activa las resistencias pull-up internas únicamente
     * en las cuatro columnas.
     *
     * En WPU:
     * 0 = pull-up desactivada.
     * 1 = pull-up activada.
     */
    *(keypad->wpu) =
        (uint8_t)(*(keypad->wpu) |
                  KEYPAD_COLUMN_MASK);

    /*
     * Estado inicial del teclado:
     * todas las filas permanecen en nivel alto.
     */
    Keypad_AllRowsHigh(keypad);
}


/*
 * Lee una tecla aplicando antirrebote y espera de liberación.
 */
char Keypad_Read(Keypad *keypad)
{
    char detected_key;
    char confirmed_key;

    /*
     * Realiza el primer barrido del teclado.
     */
    detected_key = Keypad_Scan(keypad);

    /*
     * Si no existe ninguna tecla presionada, retorna inmediatamente.
     */
    if(detected_key == KEYPAD_NO_KEY)
    {
        return KEYPAD_NO_KEY;
    }

    /*
     * Espera para reducir el efecto del rebote mecánico generado
     * al presionar la tecla.
     */
    __delay_ms(KEYPAD_DEBOUNCE_MS);

    /*
     * Realiza una segunda lectura para confirmar la pulsación.
     */
    confirmed_key = Keypad_Scan(keypad);

    /*
     * La lectura se considera inválida si:
     *
     * - La tecla dejó de estar presionada.
     * - Se detectó una tecla diferente.
     */
    if(confirmed_key != detected_key)
    {
        return KEYPAD_NO_KEY;
    }

    /*
     * Espera hasta que el usuario libere la tecla.
     *
     * Mientras exista alguna tecla presionada, la función permanece
     * dentro de este ciclo.
     *
     * Este comportamiento evita que una pulsación prolongada sea
     * interpretada varias veces.
     */
    while(Keypad_Scan(keypad) != KEYPAD_NO_KEY)
    {
        /*
         * Espera de liberación.
         */
    }

    /*
     * Retardo adicional para reducir el rebote producido
     * al soltar la tecla.
     */
    __delay_ms(KEYPAD_DEBOUNCE_MS);

    /*
     * Retorna una única vez el carácter detectado.
     */
    return detected_key;
}
