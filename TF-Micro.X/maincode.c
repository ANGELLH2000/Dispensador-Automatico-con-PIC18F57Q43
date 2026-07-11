#include <xc.h>
#include <stdint.h>

#include "cabecera.h"
#include "funcionesGenerales.h"
    
#include "LIB_UART.h"
/*==============================================================================
 * CONFIGURACIÓN GENERAL DEL MICROCONTROLADOR
 *============================================================================*/
volatile uint8_t rx_buffer[50]; 
volatile uint8_t rx_indice = 0; 
volatile uint8_t flag_paquete_listo = 0; 
volatile uint8_t longitud_esperada = 0; // ¡Nueva variable para hacer el sistema dinámico!
uint8_t datos[40];
void configuro(void)
{
    /* Inicialización de periféricos */
    config_perifericos();
    /* Configuración del oscilador interno a 64 MHz */
    OSCCON1 = 0x60;
    OSCFRQ  = 0x06;
    OSCEN   = 0x40;
    ANSELFbits.ANSELF1 = 0; // RF1 (RX) Digital
    ANSELFbits.ANSELF0 = 0; // RF0 (TX) Digital
    TRISFbits.TRISF1 = 1;   // RX como Entrada
    TRISFbits.TRISF0 = 0;   // TX como Salida
    U1_INIT(BAUD_9600);
     //configuracion de las interrupciones
    PIE4bits.U1RXIE = 1;
    PIR4bits.U1RXIF = 0;
    INTCON0bits.GIE = 1;
    
    
   
    /* Verificación inicial */
    //SubProceso_CondicionesIniciales();
}


/*==============================================================================
 * PROCESAMIENTO DEL PAQUETE (Usando Switch-Case)
 *============================================================================*/
void uart_serial(void)
{
    
    if (flag_paquete_listo == 1) {
        // Calculamos dónde cayeron el Checksum y el byte de Fin (0x0A)
        uint8_t longitud = rx_buffer[2];
        uint8_t pos_chk  = 3 + longitud;
        uint8_t pos_fin  = pos_chk + 1;
        
        // 1. Verificamos que el paquete termine correctamente en 0x0A
        if (rx_buffer[pos_fin] == 0x0A) 
        {
            uint8_t checksum_calculado = 0;

            for(int i = 0; i < longitud; i++) 
            {
                checksum_calculado += rx_buffer[3 + i];
            }

            // 3. Validamos si el Checksum del PC coincide con el que calculamos
            if (checksum_calculado == rx_buffer[pos_chk]) 
            {
            // Evaluamos el PRIMER byte de la cabecera para decidir qué comando es
                switch (rx_buffer[1]) {

                    // =======================================================
                    // CASO 1: COMANDO EEPROM (Cabecera 0xAA)
                    // =======================================================
                    case 0x56:
                        // Confirmamos el segundo byte
                        if (rx_buffer[2] == 0x01 && rx_buffer[3] == 0x52) {
                            // ¡PAQUETE CORRECTO! -> Enviar estado de la memoria
                                DataEEPROM(datos);
                                Enviar_Trama_Data(datos);

                        }
                        break; // Fin del caso AA
                    // =======================================================
                    // CASO 2: COMANDO ESCRITURA DE HORARIOS (Cabecera 0x15)
                    // =======================================================
                    case 0x15:
                        // Confirmamos el segundo byte
                        if (rx_buffer[2] == 0x04 ) {
                            // Se Agrega el Nuevo Horario
                            // [3]=> Horario [4]=> Hora [5]=> Minuto [6]=> Compartimento
                            Funcion_AgregarHorario(rx_buffer[4],rx_buffer[5],rx_buffer[6],rx_buffer[3]-1);
                            //Confirmando
                            U1_BYTE_SEND(rx_buffer[4]); U1_BYTE_SEND(rx_buffer[5]);    
                            U1_BYTE_SEND(rx_buffer[6]); U1_BYTE_SEND(rx_buffer[3]-1);  
                            U1_BYTE_SEND(0x07); U1_BYTE_SEND(0x0A);  
                        }
                        break; // Fin del caso 0x15  
                    // =======================================================
                    // CASO 3: COMANDO ESCRITURA DE PASTILLEROS (Cabecera 0x07)
                    // =======================================================
                    case 0x07:
                        // Confirmamos el segundo byte
                        if (rx_buffer[2] == 0x02 ) {
                            // Se Agrega el Nuevo Horario
                            // [3]=> Cantidad [4]=> Compartimento
                            Funcion_AgregarPastillas(rx_buffer[4],rx_buffer[3]);
                            //Confirmando
                            U1_BYTE_SEND(0xAA); U1_BYTE_SEND(0x15);    
                            U1_BYTE_SEND(0x01); U1_BYTE_SEND(0x07);  
                            U1_BYTE_SEND(0x07); U1_BYTE_SEND(0x0A);  
                        }
                        break; // Fin del caso 0x15  
                    // =======================================================
                    // POR DEFECTO: CABECERA DESCONOCIDA
                    // =======================================================
                    default:
                        // Si llega un byte que no es ni AA ni BB en la posición 0,
                        // simplemente lo ignoramos (la interrupción ya lo filtró de todos modos).
                        // Datos inválidos -> Enviar NACK de error
                        U1_BYTE_SEND(0xAA); U1_BYTE_SEND(0x55);
                        U1_BYTE_SEND(0x01); U1_BYTE_SEND(0x06); U1_BYTE_SEND(0x06);
                        break;
                }
            }
        
        }
        // Bajar la bandera para poder recibir el siguiente paquete
        flag_paquete_listo = 0; 
    }   
}
/*==============================================================================
 * FUNCIÓN PRINCIPAL
 *============================================================================*/
void main(void)
{
    configuro();
    while (1)
    {
        PantallaGeneral();
        uart_serial();
        
    }
}

/*==============================================================================
 * RUTINA DE INTERRUPCIÓN (Atrapa bytes al vuelo)
 *============================================================================*/
void __interrupt(irq(IRQ_U1RX)) U1RX_ISR(void){
    PIR4bits.U1RXIF = 0;
    uint8_t dato_entrante = U1RXB;
    
    // ELIMINADO: U1_BYTE_SEND(0x01); <-- NUNCA imprimir en RX_ISR
    
    if (flag_paquete_listo) return; 

    // Estado 0
    if (rx_indice == 0) {
        if (dato_entrante == 0xAA ) {
            rx_buffer[0] = dato_entrante;
            rx_indice = 1;
        }
        return;
    }

    // Estado 1
    if (rx_indice == 1) {
        if (dato_entrante == 0x55 || 
            dato_entrante == 0x56 || 
            dato_entrante == 0x15 || 
            dato_entrante == 0x07 ||
            dato_entrante == 0x26) {
            rx_buffer[1] = dato_entrante;
            rx_indice = 2;
        } else {
            rx_indice = 0; 
        }
        return;
    }

    // Estado 2
    if (rx_indice == 2) {
        rx_buffer[2] = dato_entrante;
        longitud_esperada = 5 + dato_entrante; 
        
        if (longitud_esperada > 15) rx_indice = 0; 
        else rx_indice = 3;
        
        return;
    }

    // Estado 3
    rx_buffer[rx_indice] = dato_entrante;
    rx_indice++;

    if (rx_indice >= longitud_esperada) {
        flag_paquete_listo = 1; 
        rx_indice = 0;          
    }
}