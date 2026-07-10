#include <xc.h>
#include <stdint.h>

#include "cabecera.h"
#include "funcionesGenerales.h"
    
#include "LIB_UART.h"
/*==============================================================================
 * CONFIGURACIÓN GENERAL DEL MICROCONTROLADOR
 *============================================================================*/

void configuro(void)
{
    /* Configuración del oscilador interno a 32 MHz */
    OSCCON1 = 0x60;
    OSCFRQ  = 0x06;
    OSCEN   = 0x40;
    U1_INIT(BAUD_9600);
    /* Inicialización de periféricos */
    config_perifericos();
   
    /* Verificación inicial */
    //SubProceso_CondicionesIniciales();
    
    
    
}

/*==============================================================================
 * FUNCIÓN PRINCIPAL
 *============================================================================*/

void main(void)
{
    uint8_t datos[40];
    configuro();
    while (1)
    {
        DataEEPROM(datos);
        Enviar_Trama_Data(datos);
        PantallaGeneral();
        __delay_ms(500);
    }
}
