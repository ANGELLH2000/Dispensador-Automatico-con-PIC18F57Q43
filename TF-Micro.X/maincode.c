#include <xc.h>
#include "cabecera.h"
#include "ws2812b.h"
#include "irsensor.h"
LED_WS2812B tira1;
IRsensor sensor_ir;


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
    WS2812B_Init(&tira1, 3);
    IRSensor_Init(&sensor_ir,&PORTD,&TRISD,&ANSELD,0x02);
    
   
   
    while(1)
    {
       if(IRSensor_ReadActiveLow(&sensor_ir))
        {
           WS2812B_RGB(&tira1, 100, 0, 0);
        }
        else
        {
            WS2812B_RGB(&tira1, 0, 100, 0);
        }
       __delay_ms(100);
    }
}
