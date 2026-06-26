/**
 * @file main.c
 * @brief Lectura del RTC DS1307 y visualización en LCD I2C.
 *
 * Microcontrolador: PIC18F57Q43.
 * Compilador: MPLAB XC8.
 *
 * El LCD y el DS1307 comparten el mismo bus:
 *
 * RC3 -> SCL
 * RC4 -> SDA
 *
 * Esta prueba es solamente de lectura.
 * No se modifica la fecha ni la hora almacenada en el RTC.
 */

#include <xc.h>
#include <stdint.h>

#include "cabecera.h"
#include "I2C.h"
#include "LCD_I2C.h"
#include "DS1307.h"


//==========================================================
// Variables globales
//==========================================================

/**
 * Estructura donde se guardará la información leída
 * desde el DS1307.
 */
static DS1307_DateTime fechaHora;


/**
 * Último resultado de la comunicación con el RTC.
 *
 * Puede observarse desde el depurador de MPLAB X.
 */
volatile I2C_Status estadoRTC = I2C_OK;


//==========================================================
// Configuración general del microcontrolador
//==========================================================

static void configuro(void)
{
    /*
     * HFINTOSC como fuente del reloj del sistema.
     * Divisor 1:1.
     */
    OSCCON1 = 0x60u;

    /*
     * HFINTOSC = 32 MHz.
     *
     * _XTAL_FREQ debe tener este mismo valor
     * dentro de cabecera.h.
     */
    OSCFRQ = 0x06u;

    /*
     * Habilita HFINTOSC.
     */
    OSCEN = 0x40u;
}


//==========================================================
// Funciones auxiliares para el LCD
//==========================================================

/**
 * @brief Escribe un número utilizando siempre dos dígitos.
 *
 * Ejemplos:
 *
 * 5  -> 05
 * 18 -> 18
 */
static void LCD_EscribirDosDigitos(uint8_t numero)
{
    LCD_I2C_WriteChar(
        (char)('0' + (numero / 10u))
    );

    LCD_I2C_WriteChar(
        (char)('0' + (numero % 10u))
    );
}


/**
 * @brief Escribe un número utilizando cuatro dígitos.
 *
 * Ejemplo:
 *
 * 2026 -> 2026
 */
static void LCD_EscribirCuatroDigitos(uint16_t numero)
{
    LCD_I2C_WriteChar(
        (char)('0' + ((numero / 1000u) % 10u))
    );

    LCD_I2C_WriteChar(
        (char)('0' + ((numero / 100u) % 10u))
    );

    LCD_I2C_WriteChar(
        (char)('0' + ((numero / 10u) % 10u))
    );

    LCD_I2C_WriteChar(
        (char)('0' + (numero % 10u))
    );
}


//==========================================================
// Visualización de la fecha y hora
//==========================================================

/**
 * @brief Muestra la fecha y hora recibida desde el DS1307.
 *
 * Formato de 24 horas:
 *
 * Hora 14:35:20
 * 26/06/2026
 *
 * Formato de 12 horas:
 *
 * Hora 02:35:20 PM
 * 26/06/2026
 */
static void LCD_MostrarFechaHora(
    const DS1307_DateTime *datos
)
{
    //======================================================
    // Primera fila: hora
    //======================================================

    LCD_I2C_SetCursor(1u, 0u);

    LCD_I2C_WriteString("Hora ");

    LCD_EscribirDosDigitos(datos->hours);
    LCD_I2C_WriteChar(':');

    LCD_EscribirDosDigitos(datos->minutes);
    LCD_I2C_WriteChar(':');

    LCD_EscribirDosDigitos(datos->seconds);


    /*
     * Completa las dieciséis posiciones del LCD.
     */
    if (datos->mode_12h != 0u)
    {
        LCD_I2C_WriteChar(' ');

        if (datos->is_pm != 0u)
        {
            LCD_I2C_WriteString("PM");
        }
        else
        {
            LCD_I2C_WriteString("AM");
        }
    }
    else
    {
        LCD_I2C_WriteString("   ");
    }


    //======================================================
    // Segunda fila: fecha
    //======================================================

    LCD_I2C_SetCursor(2u, 0u);

    LCD_EscribirDosDigitos(datos->date);
    LCD_I2C_WriteChar('/');

    LCD_EscribirDosDigitos(datos->month);
    LCD_I2C_WriteChar('/');

    LCD_EscribirCuatroDigitos(datos->year);

    /*
     * Completa las posiciones restantes.
     */
    LCD_I2C_WriteString("      ");
}


//==========================================================
// Mensajes de estado
//==========================================================

/**
 * @brief Indica que el RTC no respondió en el bus.
 */
static void LCD_MostrarRTCDesconectado(void)
{
    LCD_I2C_SetCursor(1u, 0u);
    LCD_I2C_WriteString("RTC no conectado");

    LCD_I2C_SetCursor(2u, 0u);
    LCD_I2C_WriteString("LCD sigue activo");
}


/**
 * @brief Indica que el bit CH del DS1307 está en uno.
 *
 * La librería es solamente de lectura, por lo que no modifica
 * este bit ni inicia el oscilador.
 */
static void LCD_MostrarRelojDetenido(void)
{
    LCD_I2C_SetCursor(1u, 0u);
    LCD_I2C_WriteString("Reloj detenido  ");

    LCD_I2C_SetCursor(2u, 0u);
    LCD_I2C_WriteString("CH esta en 1    ");
}


/**
 * @brief Indica que el RTC respondió, pero los registros
 * contienen valores fuera de los rangos permitidos.
 */
static void LCD_MostrarDatosInvalidos(void)
{
    LCD_I2C_SetCursor(1u, 0u);
    LCD_I2C_WriteString("RTC sin ajustar ");

    LCD_I2C_SetCursor(2u, 0u);
    LCD_I2C_WriteString("Datos invalidos ");
}


//==========================================================
// Programa principal
//==========================================================

void main(void)
{
    I2C_Status estadoLCD;

    /*
     * Estado actual de la pantalla:
     *
     * 0 -> mensaje inicial.
     * 1 -> RTC desconectado.
     * 2 -> reloj detenido.
     * 3 -> datos inválidos.
     * 4 -> fecha y hora mostradas.
     *
     * Se utiliza para evitar limpiar y reescribir
     * continuamente los mensajes de error.
     */
    uint8_t estadoPantalla = 0u;


    //======================================================
    // Configuración del microcontrolador
    //======================================================

    configuro();

    /*
     * Espera para estabilización del sistema.
     */
    __delay_ms(20);


    //======================================================
    // Inicialización del bus I2C
    //======================================================

    /*
     * El bus se inicializa una sola vez.
     *
     * Tanto el LCD como el DS1307 utilizan este mismo
     * periférico I2C1.
     */
    I2C1_Init();


    //======================================================
    // Inicialización del LCD
    //======================================================

    /*
     * Dirección de siete bits del backpack LCD.
     *
     * PCF8574_7 = 0x27.
     *
     * Si tu LCD utiliza 0x3F, reemplazar por:
     *
     * PCF8574A_7
     */
    estadoLCD = LCD_I2C_SetAddress(PCF8574_7);


    if (estadoLCD == I2C_OK)
    {
        estadoLCD = LCD_I2C_Init();
    }


    /*
     * Solo se detiene el programa si falla el LCD.
     *
     * La ausencia del RTC no detendrá el programa.
     */
    if (estadoLCD != I2C_OK)
    {
        while (1)
        {
            /*
             * Colocar un breakpoint aquí y observar
             * estadoLCD.
             */
        }
    }


    /*
     * Asegura que la iluminación del LCD esté activa.
     */
    LCD_I2C_Backlight(LCD_I2C_ON);


    //======================================================
    // Mensaje inicial
    //======================================================

    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1u, 0u);
    LCD_I2C_WriteString("Sistema iniciado");

    LCD_I2C_SetCursor(2u, 0u);
    LCD_I2C_WriteString("Bus I2C activo  ");

    __delay_ms(1000);

    LCD_I2C_Clear();


    //======================================================
    // Ciclo principal
    //======================================================

    while (1)
    {
        /*
         * Lee los registros del DS1307.
         *
         * La librería convierte automáticamente los valores
         * BCD a decimal.
         */
        estadoRTC = DS1307_ReadDateTime(
            &fechaHora
        );


        //==================================================
        // El DS1307 respondió
        //==================================================

        if (estadoRTC == I2C_OK)
        {
            /*
             * CH = 1:
             * el oscilador está detenido.
             */
            if (fechaHora.clock_running == 0u)
            {
                if (estadoPantalla != 2u)
                {
                    LCD_I2C_Clear();
                    LCD_MostrarRelojDetenido();

                    estadoPantalla = 2u;
                }
            }

            /*
             * Los registros contienen valores inválidos.
             */
            else if (fechaHora.data_valid == 0u)
            {
                if (estadoPantalla != 3u)
                {
                    LCD_I2C_Clear();
                    LCD_MostrarDatosInvalidos();

                    estadoPantalla = 3u;
                }
            }

            /*
             * Fecha y hora válidas.
             */
            else
            {
                /*
                 * Si antes había un mensaje de error,
                 * limpia la pantalla una sola vez.
                 */
                if (estadoPantalla != 4u)
                {
                    LCD_I2C_Clear();

                    estadoPantalla = 4u;
                }

                /*
                 * Actualiza únicamente los caracteres.
                 *
                 * No se borra el LCD en cada lectura para
                 * evitar parpadeos.
                 */
                LCD_MostrarFechaHora(
                    &fechaHora
                );
            }
        }


        //==================================================
        // El DS1307 no respondió
        //==================================================

        else
        {
            /*
             * Puede deberse a:
             *
             * - RTC desconectado.
             * - RTC sin alimentación.
             * - Dirección no reconocida.
             * - Error de comunicación.
             *
             * El LCD permanece funcionando.
             */
            if (estadoPantalla != 1u)
            {
                LCD_I2C_Clear();
                LCD_MostrarRTCDesconectado();

                estadoPantalla = 1u;
            }
        }


        /*
         * El RTC no necesita ser leído continuamente
         * a máxima velocidad.
         */
        __delay_ms(250);
    }
}