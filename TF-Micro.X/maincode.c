#include <xc.h>
#include "cabecera.h"
#include "motor_paso.h"

Stepper motor1;
Stepper motor2;

void config(void) {
    //conf mod de oscilador
    OSCCON1 = 0x60;
    OSCFRQ  = 0x07;
    OSCEN = 0x40;
    //Entradas y Salidas
    
}
void main(void) {
    config();
   /*
     * Inicialización del motor 1.
     *
     * Motor 1 conectado al puerto D, parte baja:
     * RD0, RD1, RD2 y RD3.
     */
    Stepper_Init(&motor1, &LATD, &TRISD, &ANSELD, STEPPER_LOW_NIBBLE);

    /*
     * Inicialización del motor 2.
     *
     * Motor 2 conectado al puerto D, parte alta:
     * RD4, RD5, RD6 y RD7.
     */
    Stepper_Init(&motor2, &LATD, &TRISD, &ANSELD, STEPPER_HIGH_NIBBLE);

    while(1)
    {
        /*
         * Ejecuta una vuelta completa del motor 1 en sentido horario.
         */
        Stepper_fullTurn_CW(&motor1);

        __delay_ms(1000);

        /*
         * Ejecuta una vuelta completa del motor 2 en sentido horario.
         */
        Stepper_fullTurn_CW(&motor2);

        __delay_ms(1000);

        /*
         * Ejecuta una vuelta completa del motor 1 en sentido antihorario.
         */
        Stepper_fullTurn_CCW(&motor1);

        __delay_ms(1000);

        /*
         * Ejecuta una vuelta completa del motor 2 en sentido antihorario.
         */
        Stepper_fullTurn_CCW(&motor2);

        __delay_ms(1000);

        /*
         * Desactiva ambos motores.
         */
        Stepper_Off(&motor1);
        Stepper_Off(&motor2);

        __delay_ms(1000);
    }
}
