#include <xc.h>
#include <stdint.h>

#include "cabecera.h"
#include "funcionesGenerales.h"
    
#include "LIB_UART.h"
/*==============================================================================
 * CONFIGURACIÓN GENERAL DEL MICROCONTROLADOR
 *============================================================================*/
// Prototipo de la función de envío
void Enviar_Trama_Data(unsigned char *buffer);
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
    configuro();
    unsigned char datos_prueba[40];
    
    // Llenamos el arreglo con algunos valores de prueba para verificar en la PC
    datos_prueba[0] = 4;   // Horarios_dispensados = 4
    datos_prueba[1] = 2;   // Total_horarios_guardados = 2
    datos_prueba[2] = 80;  // Cantidad_pastillas_total = 80 (0x50 en Hex)
    datos_prueba[3] = 3;   // Cantidad_compartimientos = 3
    datos_prueba[4] = 0;   // Bandera_error_sistema = 0 (Sin error)
    
    // El resto de la tabla (del índice 5 al 39) lo rellenamos con ceros por ahora
    for (int i = 5; i < 40; i++) {
        datos_prueba[i] = 0;
    }

    while (1)
    {
        Enviar_Trama_Data(datos_prueba);
        //PantallaGeneral();
        //U1_STRING_SEND("Apuren que nos quedamos sin tiempo!");
        //U1_NEWLINE();  
        __delay_ms(2000);
    }
}
void Enviar_Trama_Data(unsigned char *buffer) {
    unsigned char checksum = 0;
    U1_BYTE_SEND(0xFF);
    // Entrada 1: Enviar los 2 bytes de la cabecera fija (Header)
    U1_BYTE_SEND(170);
    U1_BYTE_SEND(0x55);
    
    // Entrada 2: Enviar el tamaño del contenido (40 bytes -> 0x28 en Hex)
    U1_BYTE_SEND(0x28); 
    
    // Entrada 3: Enviar secuencialmente los 40 datos y acumular el Checksum
    for (int i = 0; i < 40; i++) {
        U1_BYTE_SEND(buffer[i]);
        checksum += buffer[i]; // El desborde de 8 bits realiza el módulo 256 automáticamente
    }
    
    // Entrada 4: Enviar el Checksum calculado
    U1_BYTE_SEND(checksum);
    
    // Entrada 5: Enviar el byte de cierre (Footer)
    U1_BYTE_SEND(0x0A); // Salto de línea (\n)
}