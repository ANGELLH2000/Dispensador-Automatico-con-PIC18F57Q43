//Librer?a LIB_UART desarrollada por Kalun Lau
//Curso de Microcontroladores
//Universidad Peruana de Ciencias Aplicadas
//Ultima edicion 10/06/2024

//Changelog:
//10/06/2024: Se esta considerando el uso del U1
//como per?ferico a emplearse en la comunicacion serial.
//La velocidad de comunicacion establecida es 9600
//La frecuencia de trabajo es de 32MHz
//Tener en cuenta que para la recepcion de datos se esta
//empleando interrupciones del USART para recepcion

//Instrucciones para usar la librer?a
//1.Llamar a U1_INIT() para configurar el UART U1
//2.Para la parte de recepcion deberan de habilitar
//la funcion de interrupcion en el codigo principal

#include <xc.h>
#include <string.h>
#include "LIB_UART.h"

void U1_INIT(unsigned int velocidad){
    //configuracion del UART
    U1BRGH = (velocidad >>8) & 0x00FF; //revisar defines en LIB_UART.h
    U1BRGL = velocidad & 0x00FF;  //revisar defines en LIB_UART.h           
    U1CON0 = 0x30;          //TX enabled, RX enabled
    U1CON1 = 0x80;
    U1CON2 = 0x00;
    RF0PPS = 0x20;           //RF0 esta conectado a UART1TX
    U1RXPPS = 0x29;             //RF1 esta conectado a UART1RX
   
}

/*funcion para enviar un dato de 8 bits*/
void U1_BYTE_SEND(unsigned char dato){
    U1TXB = dato;
    while(U1ERRIRbits.TXMTIF == 0);
}

/*funcion para enviar una cadena de caracteres*/
void U1_STRING_SEND(const char *cadena)
{
    unsigned char tam;
    tam = strlen(cadena);
	unsigned char i = 0;
	for(i = 0; i<tam; i++)
	{
		U1_BYTE_SEND(cadena[i]);
	}
}


void Enviar_Trama_Data(unsigned char *buffer) {
    unsigned char checksum = 0;
    U1_BYTE_SEND(0xFF);//
    // Entrada 1: Enviar los 2 bytes de la cabecera fija (Header)
    U1_BYTE_SEND(0xAA);//0xAA
    U1_BYTE_SEND(0x55);//Bit de Confirmacion
    
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



void U1_NEWLINE(void){
    U1_BYTE_SEND(0x0A);
    U1_BYTE_SEND(0x0D);
}

