/*
 * File:   maincode00.c
 * Author: ALUMNOS
 *
 * Descripción:
 * Ejemplo de uso de la librería CNY70 con dos sensores analógicos:
 *
 * Sensor 1: RA3 / AN3
 * Sensor 2: RA4 / AN4
 *
 * La librería descarga el capacitor interno del ADC solamente cuando
 * cambia de canal analógico.
 */

#include <xc.h>
#include <stdint.h>
#include "cabecera.h"
#include "LCD.h"
#include "CNY70.h"

#define _XTAL_FREQ 64000000UL

CNY70 sensorA3;
CNY70 sensorA4;

unsigned int valor_A3 = 0;
unsigned int valor_A4 = 0;

void configuro(void)
{
    //================================================
    // Oscilador general del PIC
    // Esto no pertenece a la librería CNY70.
    //================================================
    OSCCON1 = 0x60;
    OSCFRQ  = 0x08;
    OSCEN   = 0x40;

    //================================================
    // LCD en puerto D
    //================================================
    TRISD  = 0x00;
    ANSELD = 0x00;

    //================================================
    // Inicialización general del ADC desde la librería
    //================================================
    CNY70_ADC_Init_FOSC64();

    //================================================
    // Inicialización de sensores CNY70
    //================================================

    // Sensor conectado en RA3 / AN3
    CNY70_Init(&sensorA3, &TRISA, &ANSELA, 0x08, 0x03);

    // Sensor conectado en RA4 / AN4
    CNY70_Init(&sensorA4, &TRISA, &ANSELA, 0x10, 0x04);
}

void LCD_init(void)
{
    LCD_CONFIG();
    __delay_ms(16);
    BORRAR_LCD();
    CURSOR_HOME();
    CURSOR_ONOFF(OFF);
}

void main(void)
{
    configuro();
    LCD_init();

    POS_CURSOR(1, 0);
    ESCRIBE_MENSAJE("CNY70 x2", 8);

    POS_CURSOR(2, 0);
    ESCRIBE_MENSAJE("A3 y A4", 7);

    __delay_ms(1500);
    BORRAR_LCD();

    while(1)
    {
        //================================================
        // Primera lectura: sensor en RA3 / AN3
        //================================================
        valor_A3 = CNY70_Read(&sensorA3);

        //================================================
        // Segunda lectura: sensor en RA4 / AN4
        // Aquí la librería detecta que cambió de AN3 a AN4,
        // por lo tanto descarga el capacitor antes de leer.
        //================================================
        valor_A4 = CNY70_Read(&sensorA4);

        //================================================
        // Mostrar lectura de RA3
        //================================================
        POS_CURSOR(1, 0);
        ESCRIBE_MENSAJE("A3:", 3);

        ENVIA_CHAR((valor_A3 / 10000) + 0x30);
        ENVIA_CHAR(((valor_A3 % 10000) / 1000) + 0x30);
        ENVIA_CHAR(((valor_A3 % 1000) / 100) + 0x30);
        ENVIA_CHAR(((valor_A3 % 100) / 10) + 0x30);
        ENVIA_CHAR((valor_A3 % 10) + 0x30);

        ESCRIBE_MENSAJE("   ", 3);

        //================================================
        // Mostrar lectura de RA4
        //================================================
        POS_CURSOR(2, 0);
        ESCRIBE_MENSAJE("A4:", 3);

        ENVIA_CHAR((valor_A4 / 10000) + 0x30);
        ENVIA_CHAR(((valor_A4 % 10000) / 1000) + 0x30);
        ENVIA_CHAR(((valor_A4 % 1000) / 100) + 0x30);
        ENVIA_CHAR(((valor_A4 % 100) / 10) + 0x30);
        ENVIA_CHAR((valor_A4 % 10) + 0x30);

        ESCRIBE_MENSAJE("   ", 3);

        __delay_ms(200);
    }
}