#include <xc.h>
#include <stdint.h>

#include "cabecera.h"
#include "I2C.h"
#include "LCD_I2C.h"


volatile I2C_Status estado = I2C_OK;


void configuro(void)
{
    /*
     * HFINTOSC como fuente del reloj.
     * Divisor 1:1.
     */
    OSCCON1 = 0x60u;

    /*
     * HFINTOSC a 32 MHz.
     */
    OSCFRQ = 0x06u;

    /*
     * Habilita HFINTOSC.
     */
    OSCEN = 0x40u;
}


void main(void)
{
    configuro();

    __delay_ms(20);

    /*
     * Inicializa el bus I2C una sola vez.
     */
    I2C1_Init();

    /*
     * Prueba primero con 0x27.
     */
    estado = LCD_I2C_SetAddress(PCF8574_7);

    if (estado == I2C_OK)
    {
        estado = LCD_I2C_Init();
    }

    if (estado != I2C_OK)
    {
        while (1)
        {
            /*
             * Coloca un breakpoint aquí.
             *
             * Si estado = I2C_ERROR_NACK,
             * probablemente la dirección del LCD es incorrecta.
             */
        }
    }

    /*
     * Fuerza el encendido del backlight.
     */
    estado = LCD_I2C_Backlight(LCD_I2C_ON);

    /*
     * Limpia y escribe un mensaje sencillo.
     */
    estado = LCD_I2C_Clear();

    estado = LCD_I2C_SetCursor(1u, 0u);
    estado = LCD_I2C_WriteString("PRUEBA LCD");

    estado = LCD_I2C_SetCursor(2u, 0u);
    estado = LCD_I2C_WriteString("I2C FUNCIONA");

    while (1)
    {
    }
}