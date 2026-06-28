#include <xc.h>
#include <stdint.h>

#include "cabecera.h"
#include "funcionesGenerales.h"

/*==============================================================================
 * CONFIGURACIÓN GENERAL DEL MICROCONTROLADOR
 *============================================================================*/

void configuro(void)
{
    /* Configuración del oscilador interno a 32 MHz */
    OSCCON1 = 0x60;
    OSCFRQ  = 0x06;
    OSCEN   = 0x40;

    /* Inicialización de periféricos */
    config_perifericos();

    /* Verificación inicial */
    verificar_condiciones_iniciales();
    
}

/*==============================================================================
 * FUNCIÓN PRINCIPAL
 *============================================================================*/

void main(void)
{
    configuro();

    while (1)
    {

    }
}