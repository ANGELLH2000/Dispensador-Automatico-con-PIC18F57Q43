#include <xc.h>
#include <stdint.h>
#include "cabecera.h"
#include "EPROM_DFM.h"
#include "LCD.h"

//#define _XTAL_FREQ 64000000UL //Se define en cabecera

#define ADDR_DATO_0     0
#define ADDR_DATO_1     1

#define BTN_CAMBIAR     PORTCbits.RC0
#define BTN_GRABAR      PORTCbits.RC1
#define BTN_POSICION    PORTCbits.RC3

unsigned char valor0 = 0;
unsigned char valor1 = 0;
unsigned char posicion = 0;

void configuro(void)
{
    //================================================
    // Oscilador general del PIC
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
    // Botones en RC0, RC1 y RC3
    //================================================
    TRISCbits.TRISC0 = 1;      // RC0 entrada
    TRISCbits.TRISC1 = 1;      // RC1 entrada
    TRISCbits.TRISC3 = 1;      // RC3 entrada

    ANSELCbits.ANSELC0 = 0;    // RC0 digital
    ANSELCbits.ANSELC1 = 0;    // RC1 digital
    ANSELCbits.ANSELC3 = 0;    // RC3 digital

    WPUCbits.WPUC0 = 1;        // Pull-up interno RC0
    WPUCbits.WPUC1 = 1;        // Pull-up interno RC1
    WPUCbits.WPUC3 = 1;        // Pull-up interno RC3
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
    ESCRIBE_MENSAJE("  MEMORIA EPROM  ", 16);
    __delay_ms(2000);
    //================================================
    // Leer valores guardados en EEPROM / DFM
    //================================================
    valor0 = EEPROM_ReadByte(ADDR_DATO_0);
    valor1 = EEPROM_ReadByte(ADDR_DATO_1);

    if(valor0 == 0xFF)
    {
        valor0 = 0;
    }

    if(valor1 == 0xFF)
    {
        valor1 = 0;
    }

    BORRAR_LCD();

    while(1)
    {
        //================================================
        // Mostrar valores de ambas direcciones
        //================================================
        POS_CURSOR(1, 0);
        ESCRIBE_MENSAJE("P0:", 3);

        ENVIA_CHAR((valor0 / 100) + 0x30);
        ENVIA_CHAR(((valor0 % 100) / 10) + 0x30);
        ENVIA_CHAR((valor0 % 10) + 0x30);

        ESCRIBE_MENSAJE(" P1:", 4);

        ENVIA_CHAR((valor1 / 100) + 0x30);
        ENVIA_CHAR(((valor1 % 100) / 10) + 0x30);
        ENVIA_CHAR((valor1 % 10) + 0x30);

        ESCRIBE_MENSAJE(" ", 1);

        POS_CURSOR(2, 0);
        ESCRIBE_MENSAJE("SEL:", 4);
        ENVIA_CHAR(posicion + 0x30);

        ESCRIBE_MENSAJE("  GR:C1   ", 10);

        //================================================
        // RC0: Aumentar valor de la posición seleccionada
        //================================================
        if(BTN_CAMBIAR == 0)
        {
            __delay_ms(40);

            if(BTN_CAMBIAR == 0)
            {
                if(posicion == 0)
                {
                    valor0++;
                }
                else
                {
                    valor1++;
                }

                while(BTN_CAMBIAR == 0);
                __delay_ms(40);
            }
        }

        //================================================
        // RC1: Grabar valor en la posición seleccionada
        //================================================
        if(BTN_GRABAR == 0)
        {
            __delay_ms(40);

            if(BTN_GRABAR == 0)
            {
                if(posicion == 0)
                {
                    EEPROM_UpdateByte(ADDR_DATO_0, valor0);
                }
                else
                {
                    EEPROM_UpdateByte(ADDR_DATO_1, valor1);
                }

                BORRAR_LCD();

                POS_CURSOR(1, 0);
                ESCRIBE_MENSAJE("Dato guardado", 13);

                POS_CURSOR(2, 0);
                ESCRIBE_MENSAJE("Posicion: ", 10);
                ENVIA_CHAR(posicion + 0x30);

                __delay_ms(800);
                BORRAR_LCD();

                while(BTN_GRABAR == 0);
                __delay_ms(40);
            }
        }

        //================================================
        // RC3: Cambiar posición de memoria 0 / 1
        //================================================
        if(BTN_POSICION == 0)
        {
            __delay_ms(40);

            if(BTN_POSICION == 0)
            {
                if(posicion == 0)
                {
                    posicion = 1;
                }
                else
                {
                    posicion = 0;
                }

                while(BTN_POSICION == 0);
                __delay_ms(40);
            }
        }
    }
}