#include <xc.h>
#include <stdint.h>

#include "cabecera.h"
#include "Keypad4x4.h"
#include "LCD.h"


/*
 * Estructura asociada al teclado matricial 4x4.
 */
Keypad teclado;


/*
 * Variable utilizada para guardar la tecla detectada.
 */
char tecla;


/*
 * Configuración general del microcontrolador.
 */
void configuro(void)
{
    OSCCON1 = 0x60;
    OSCFRQ  = 0x08;
    OSCEN   = 0x40;
    TRISD  = 0x00;
    ANSELD = 0x00;
    LCD_CONFIG();
}


/*
 * Función principal.
 */
void main(void)
{
    configuro();
    
    BORRAR_LCD();
    CURSOR_HOME();
    CURSOR_ONOFF(OFF);

    /*
     * Inicializa el teclado matricial en el puerto B.
     *
     * Conexión utilizada:
     *
     * RB0 -> Fila 1
     * RB1 -> Fila 2
     * RB2 -> Fila 3
     * RB3 -> Fila 4
     *
     * RB4 -> Columna 1
     * RB5 -> Columna 2
     * RB6 -> Columna 3
     * RB7 -> Columna 4
     */
    Keypad_Init(&teclado,
                &PORTC,
                &LATC,
                &TRISC,
                &ANSELC,
                &WPUC);
    POS_CURSOR(1, 0);
    ESCRIBE_MENSAJE("Tecla:", 6);
    POS_CURSOR(2, 0);
    ENVIA_CHAR('-');

    while(1)
    {
        tecla = Keypad_Read(&teclado);

        /*
         * Comprueba si se obtuvo una tecla válida.
         */
        if(tecla != KEYPAD_NO_KEY)
        {
            POS_CURSOR(2, 0);
            ENVIA_CHAR((unsigned char)tecla);
            ESCRIBE_MENSAJE("               ", 15);
        }
    }
}
