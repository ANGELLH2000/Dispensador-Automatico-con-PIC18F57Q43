#include <xc.h>
#include <stdint.h>
#include "cabecera.h"
#include "LCD.h"
#include "CNY70.h"
#include "Libbuzzer.h"
#include "motor_paso.h"
#define _XTAL_FREQ 64000000UL

#define UMBRAL_IR 750

CNY70 sensorIR;
Buzzer buzzer1;
Stepper motor1;
unsigned int resultado_ADC = 0;
unsigned int contador = 0;

unsigned char estado_bloqueado = 0;
unsigned char sistema_listo = 0;

void configuro(void)
{
    //================================================
    // Oscilador general del PIC
    // Esto NO pertenece a la librer?a CNY70.
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
    // ADC configurado desde la librer?a
    //================================================
    CNY70_ADC_Init_FOSC64();

    //================================================
    // Sensor en RA3 / AN3
    // RA3  -> 0x08
    // AN3  -> 0x03
    //================================================
    CNY70_Init(&sensorIR, &TRISA, &ANSELA, 0x08, 0x03);
    Buzzer_Init(&buzzer1, &LATA, &TRISA, &ANSELA, 0x01);
    Stepper_Init(&motor1, &LATC, &TRISC, &ANSELC, STEPPER_LOW_NIBBLE);
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
    ESCRIBE_MENSAJE("CNY70 Lib", 9);

    POS_CURSOR(2, 0);
    ESCRIBE_MENSAJE("RA3 / AN3", 9);

    __delay_ms(1500);
    BORRAR_LCD();

    while(1)
    {
        resultado_ADC = CNY70_Read(&sensorIR);

        POS_CURSOR(1, 0);
        ESCRIBE_MENSAJE("ADC:", 4);

        ENVIA_CHAR((resultado_ADC / 10000) + 0x30);
        ENVIA_CHAR(((resultado_ADC % 10000) / 1000) + 0x30);
        ENVIA_CHAR(((resultado_ADC % 1000) / 100) + 0x30);
        ENVIA_CHAR(((resultado_ADC % 100) / 10) + 0x30);
        ENVIA_CHAR((resultado_ADC % 10) + 0x30);

        ESCRIBE_MENSAJE("   ", 3);

        POS_CURSOR(2, 0);

        if(resultado_ADC <= UMBRAL_IR)
        {
            if((estado_bloqueado == 0) && (sistema_listo == 1))
            {
                contador++;
                estado_bloqueado = 1;
            }

            ESCRIBE_MENSAJE("Pastillas: ", 11);
        }
        else
        {
            sistema_listo = 1;
            estado_bloqueado = 0;
            Buzzer_CorrectSound(&buzzer1);
            //Si deseas desactiva el comentario, sin embargo en la lcd se ve muy r?pido
            //ESCRIBE_MENSAJE("Detectando ", 11);
        }

        ENVIA_CHAR(((contador % 1000) / 100) + 0x30);
        ENVIA_CHAR(((contador % 100) / 10) + 0x30);
        ENVIA_CHAR((contador % 10) + 0x30);

        ESCRIBE_MENSAJE("   ", 3);
        Stepper_Step_CW(&motor1);
    }
}