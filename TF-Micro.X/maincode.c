#include <xc.h>
#include "cabecera.h"
#include "ws2812b.h"

LED_WS2812B tira1;

void config(void) {
    //conf mod de oscilador
    OSCCON1 = 0x60;
    OSCFRQ  = 0x07;
    OSCEN = 0x40;
    //Entradas y Salidas
    
}
void main(void)
{
    /*
     * Configuración inicial del sistema.
     *
     * Esta función debe configurar el oscilador y cualquier otro módulo
     * necesario antes de iniciar la tira WS2812B.
     */
    config();

    /*
     * Inicializa la tira WS2812B.
     *
     * En esta versión, el pin de datos se define en ws2812b.h mediante:
     *
     * WS2812B_LAT
     * WS2812B_TRIS
     * WS2812B_ANSEL
     *
     * El valor 3 indica que se controlarán 3 LEDs.
     */
    WS2812B_Init(&tira1, 3);

    while(1)
    {
        /*
         * Enciende los 3 LEDs en color rojo.
         *
         * Parámetros:
         * rojo  = 200
         * verde = 0
         * azul  = 0
         */
        WS2812B_RGB(&tira1, 200, 0, 0);
        __delay_ms(1000);

        /*
         * Enciende los 3 LEDs en color verde.
         *
         * Parámetros:
         * rojo  = 0
         * verde = 200
         * azul  = 0
         */
        WS2812B_RGB(&tira1, 0, 200, 0);
        __delay_ms(1000);

        /*
         * Enciende los 3 LEDs en color azul.
         *
         * El valor máximo permitido para cada canal RGB es 255.
         * Por eso no se debe usar 2000.
         */
        WS2812B_RGB(&tira1, 0, 0, 200);
        __delay_ms(1000);

        /*
         * Apaga todos los LEDs de la tira.
         */
        WS2812B_Clear(&tira1);
        __delay_ms(1000);
    }
}
