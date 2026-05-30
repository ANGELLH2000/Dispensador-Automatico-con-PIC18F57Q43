#include <xc.h>
#include "cabecera.h"
#include "ws2812b.h"


void config(void) {
    //conf mod de oscilador
    OSCCON1 = 0x60;
    OSCFRQ  = 0x07;
    OSCEN = 0x40;
    //Entradas y Salidas
    
}
void main(void) {
    config();
   WS2812B_Init();

    while(1)
    {
        /*
         * Apagado.
         */
        WS2812B_Clear();
        WS2812B_Show();
        __delay_ms(1000);

        /*
         * Rojo con brillo bajo.
         */
        WS2812B_SetAll(20, 0, 0);
        WS2812B_Show();
        __delay_ms(1000);

        /*
         * Verde con brillo bajo.
         */
        WS2812B_SetAll(0, 20, 0);
        WS2812B_Show();
        __delay_ms(1000);

        /*
         * Azul con brillo bajo.
         */
        WS2812B_SetAll(0, 0, 20);
        WS2812B_Show();
        __delay_ms(1000);
    }
}
