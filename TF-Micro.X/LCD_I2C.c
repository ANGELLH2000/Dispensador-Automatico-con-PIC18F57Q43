/*==========================================================
 * Archivo: LCD_I2C.c
 *==========================================================*/

/**
 * @file LCD_I2C.c
 * @brief Implementación del LCD HD44780 mediante PCF8574.
 */

#include <stddef.h>
#include <stdint.h>
#include "cabecera.h"
#include "LCD_I2C.h"


//==========================================================
// Mapeo PCF8574 hacia el LCD
//==========================================================

#define LCD_PIN_RS          0x01u
#define LCD_PIN_RW          0x02u
#define LCD_PIN_EN          0x04u
#define LCD_PIN_BACKLIGHT   0x08u


//==========================================================
// Comandos del HD44780
//==========================================================

#define LCD_CMD_CLEAR                0x01u
#define LCD_CMD_HOME                 0x02u
#define LCD_CMD_ENTRY_INCREMENT      0x06u
#define LCD_CMD_DISPLAY_CONTROL      0x08u
#define LCD_CMD_FUNCTION_4BIT_2LINE  0x28u
#define LCD_CMD_SET_CGRAM            0x40u
#define LCD_CMD_SET_DDRAM            0x80u


//==========================================================
// Bits de Display Control
//==========================================================

#define LCD_DISPLAY_BIT  0x04u
#define LCD_CURSOR_BIT   0x02u
#define LCD_BLINK_BIT    0x01u


//==========================================================
// Variables privadas
//==========================================================

/**
 * Dirección predeterminada:
 *
 * PCF8574 con A2:A0 = 111 -> 0x27.
 */
static uint8_t lcdAddress = PCF8574_7;


/**
 * Estado actual de la luz de fondo.
 */
static uint8_t lcdBacklight = LCD_PIN_BACKLIGHT;


/**
 * Conserva independientemente los estados de:
 *
 * - Display.
 * - Cursor.
 * - Parpadeo.
 */
static uint8_t lcdDisplayControl = LCD_DISPLAY_BIT;


//==========================================================
// Prototipos privados
//==========================================================

static I2C_Status LCD_I2C_ExpanderWrite(uint8_t value);

static I2C_Status LCD_I2C_WriteNibble(
    uint8_t nibble,
    uint8_t rs
);

static I2C_Status LCD_I2C_SendByte(
    uint8_t value,
    uint8_t rs
);

static I2C_Status LCD_I2C_UpdateDisplayControl(void);

static I2C_Status LCD_I2C_WriteFixedUnsigned(
    uint16_t number,
    uint8_t digits,
    uint8_t decimals
);

static char LCD_I2C_NibbleToHex(uint8_t nibble);


//==========================================================
// Funciones internas
//==========================================================

/**
 * @brief Envía directamente un byte al PCF8574.
 */
static I2C_Status LCD_I2C_ExpanderWrite(uint8_t value)
{
    return I2C1_WriteSingleByte(
        lcdAddress,
        value
    );
}


/**
 * @brief Envía un nibble y genera el pulso de Enable.
 *
 * Los dos estados de Enable se transmiten dentro de una
 * sola transacción I2C:
 *
 * 1. E = 1.
 * 2. E = 0.
 */
static I2C_Status LCD_I2C_WriteNibble(
    uint8_t nibble,
    uint8_t rs
)
{
    uint8_t base;
    uint8_t block[2];


    /*
     * Conserva solamente P4-P7.
     */
    base = (uint8_t)(nibble & 0xF0u);


    /*
     * Agrega el estado del backlight.
     */
    base |= lcdBacklight;


    /*
     * RS = 0: comando.
     * RS = 1: dato.
     */
    if (rs != 0u)
    {
        base |= LCD_PIN_RS;
    }


    /*
     * RW permanece en cero.
     *
     * La librería solamente escribe en el LCD.
     */
    block[0] = (uint8_t)(base | LCD_PIN_EN);
    block[1] = (uint8_t)(base & (uint8_t)(~LCD_PIN_EN));


    return I2C1_Write(
        lcdAddress,
        block,
        2u
    );
}


/**
 * @brief Envía un byte completo en modo de cuatro bits.
 */
static I2C_Status LCD_I2C_SendByte(
    uint8_t value,
    uint8_t rs
)
{
    uint8_t control;
    uint8_t highNibble;
    uint8_t lowNibble;
    uint8_t block[4];


    control = lcdBacklight;


    if (rs != 0u)
    {
        control |= LCD_PIN_RS;
    }


    /*
     * Nibble superior.
     */
    highNibble = (uint8_t)(value & 0xF0u);


    /*
     * Nibble inferior desplazado hacia P4-P7.
     */
    lowNibble = (uint8_t)((value << 4) & 0xF0u);


    /*
     * Pulso de Enable para el nibble superior.
     */
    block[0] = (uint8_t)(
        highNibble |
        control |
        LCD_PIN_EN
    );

    block[1] = (uint8_t)(
        highNibble |
        control
    );


    /*
     * Pulso de Enable para el nibble inferior.
     */
    block[2] = (uint8_t)(
        lowNibble |
        control |
        LCD_PIN_EN
    );

    block[3] = (uint8_t)(
        lowNibble |
        control
    );


    /*
     * Los cuatro estados se transmiten dentro de una
     * sola transacción I2C.
     */
    return I2C1_Write(
        lcdAddress,
        block,
        4u
    );
}


/**
 * @brief Actualiza display, cursor y blink.
 */
static I2C_Status LCD_I2C_UpdateDisplayControl(void)
{
    return LCD_I2C_Command(
        (uint8_t)(
            LCD_CMD_DISPLAY_CONTROL |
            lcdDisplayControl
        )
    );
}


//==========================================================
// Dirección del backpack
//==========================================================

I2C_Status LCD_I2C_SetAddress(uint8_t address7)
{
    if (address7 > 0x7Fu)
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }

    lcdAddress = address7;

    return I2C_OK;
}


uint8_t LCD_I2C_GetAddress(void)
{
    return lcdAddress;
}


//==========================================================
// Comandos y caracteres
//==========================================================

I2C_Status LCD_I2C_Command(uint8_t command)
{
    I2C_Status status;


    /*
     * RS = 0 porque se está enviando un comando.
     */
    status = LCD_I2C_SendByte(
        command,
        0u
    );


    if (status != I2C_OK)
    {
        return status;
    }


    /*
     * Clear y Home necesitan aproximadamente 1.52 ms.
     */
    if ((command == LCD_CMD_CLEAR) ||
        (command == LCD_CMD_HOME))
    {
        __delay_ms(2);
    }
    else
    {
        __delay_us(50);
    }


    return I2C_OK;
}


I2C_Status LCD_I2C_WriteChar(char character)
{
    I2C_Status status;


    /*
     * RS = 1 porque se está enviando un dato.
     */
    status = LCD_I2C_SendByte(
        (uint8_t)character,
        1u
    );


    if (status == I2C_OK)
    {
        __delay_us(50);
    }


    return status;
}


//==========================================================
// Inicialización del LCD
//==========================================================

I2C_Status LCD_I2C_Init(void)
{
    I2C_Status status;


    /*
     * Tiempo mínimo de espera después de aplicar alimentación.
     */
    __delay_ms(50u);


    lcdBacklight = LCD_PIN_BACKLIGHT;
    lcdDisplayControl = LCD_DISPLAY_BIT;


    /*
     * Coloca el expansor en un estado conocido:
     *
     * RS = 0
     * RW = 0
     * E  = 0
     * Backlight = encendido
     * D4-D7 = 0
     */
    status = LCD_I2C_ExpanderWrite(lcdBacklight);


    if (status != I2C_OK)
    {
        return status;
    }


    __delay_ms(5u);


    //======================================================
    // Entrada al modo de cuatro bits
    //======================================================

    status = LCD_I2C_WriteNibble(0x30u, 0u);

    if (status != I2C_OK)
    {
        return status;
    }

    __delay_ms(5u);


    status = LCD_I2C_WriteNibble(0x30u, 0u);

    if (status != I2C_OK)
    {
        return status;
    }

    __delay_us(150);


    status = LCD_I2C_WriteNibble(0x30u, 0u);

    if (status != I2C_OK)
    {
        return status;
    }

    __delay_us(150);


    /*
     * 0x02 selecciona definitivamente el modo de cuatro bits.
     */
    status = LCD_I2C_WriteNibble(0x20u, 0u);

    if (status != I2C_OK)
    {
        return status;
    }

    __delay_us(150);


    //======================================================
    // Configuración normal
    //======================================================

    /*
     * 0x28:
     *
     * Interfaz de cuatro bits.
     * Dos líneas.
     * Caracteres de 5x8 puntos.
     */
    status = LCD_I2C_Command(
        LCD_CMD_FUNCTION_4BIT_2LINE
    );

    if (status != I2C_OK)
    {
        return status;
    }


    /*
     * Apaga temporalmente display, cursor y blink.
     */
    lcdDisplayControl = 0x00u;

    status = LCD_I2C_UpdateDisplayControl();

    if (status != I2C_OK)
    {
        return status;
    }


    /*
     * Limpia la memoria visible.
     */
    status = LCD_I2C_Clear();

    if (status != I2C_OK)
    {
        return status;
    }


    /*
     * El cursor avanza después de cada carácter.
     */
    status = LCD_I2C_Command(
        LCD_CMD_ENTRY_INCREMENT
    );

    if (status != I2C_OK)
    {
        return status;
    }


    /*
     * Display encendido.
     * Cursor apagado.
     * Blink apagado.
     */
    lcdDisplayControl = LCD_DISPLAY_BIT;

    return LCD_I2C_UpdateDisplayControl();
}


//==========================================================
// Escritura de cadenas
//==========================================================

I2C_Status LCD_I2C_WriteString(const char *text)
{
    I2C_Status status;


    if (text == NULL)
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    while (*text != '\0')
    {
        status = LCD_I2C_WriteChar(*text);


        if (status != I2C_OK)
        {
            return status;
        }


        text++;
    }


    return I2C_OK;
}


I2C_Status LCD_I2C_WriteStringN(
    const char *text,
    uint8_t length
)
{
    uint8_t index;
    I2C_Status status;


    if (text == NULL)
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    for (index = 0u; index < length; index++)
    {
        status = LCD_I2C_WriteChar(text[index]);


        if (status != I2C_OK)
        {
            return status;
        }
    }


    return I2C_OK;
}


//==========================================================
// Posicionamiento
//==========================================================

I2C_Status LCD_I2C_SetCursor(
    uint8_t row,
    uint8_t column
)
{
    uint8_t rowAddress;


    switch (row)
    {
        case 1u:
            rowAddress = 0x00u;
            break;

        case 2u:
            rowAddress = 0x40u;
            break;

        case 3u:
            rowAddress = 0x14u;
            break;

        case 4u:
            rowAddress = 0x54u;
            break;

        default:
            return I2C_ERROR_INVALID_PARAMETER;
    }


    return LCD_I2C_Command(
        (uint8_t)(
            LCD_CMD_SET_DDRAM |
            (rowAddress + column)
        )
    );
}


//==========================================================
// Control básico
//==========================================================

I2C_Status LCD_I2C_Clear(void)
{
    return LCD_I2C_Command(LCD_CMD_CLEAR);
}


I2C_Status LCD_I2C_Home(void)
{
    return LCD_I2C_Command(LCD_CMD_HOME);
}


I2C_Status LCD_I2C_Display(uint8_t state)
{
    if (state == LCD_I2C_ON)
    {
        lcdDisplayControl |= LCD_DISPLAY_BIT;
    }
    else if (state == LCD_I2C_OFF)
    {
        lcdDisplayControl &=
            (uint8_t)(~LCD_DISPLAY_BIT);
    }
    else
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    return LCD_I2C_UpdateDisplayControl();
}


I2C_Status LCD_I2C_Cursor(uint8_t state)
{
    if (state == LCD_I2C_ON)
    {
        lcdDisplayControl |= LCD_CURSOR_BIT;
    }
    else if (state == LCD_I2C_OFF)
    {
        lcdDisplayControl &=
            (uint8_t)(~LCD_CURSOR_BIT);
    }
    else
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    return LCD_I2C_UpdateDisplayControl();
}


I2C_Status LCD_I2C_Blink(uint8_t state)
{
    if (state == LCD_I2C_ON)
    {
        lcdDisplayControl |= LCD_BLINK_BIT;
    }
    else if (state == LCD_I2C_OFF)
    {
        lcdDisplayControl &=
            (uint8_t)(~LCD_BLINK_BIT);
    }
    else
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    return LCD_I2C_UpdateDisplayControl();
}


I2C_Status LCD_I2C_Backlight(uint8_t state)
{
    if (state == LCD_I2C_ON)
    {
        lcdBacklight = LCD_PIN_BACKLIGHT;
    }
    else if (state == LCD_I2C_OFF)
    {
        lcdBacklight = 0x00u;
    }
    else
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    /*
     * Actualiza inmediatamente la salida del expansor.
     */
    return LCD_I2C_ExpanderWrite(lcdBacklight);
}


//==========================================================
// Desplazamientos
//==========================================================

I2C_Status LCD_I2C_CursorShiftLeft(void)
{
    return LCD_I2C_Command(0x10u);
}


I2C_Status LCD_I2C_CursorShiftRight(void)
{
    return LCD_I2C_Command(0x14u);
}


I2C_Status LCD_I2C_DisplayShiftLeft(void)
{
    return LCD_I2C_Command(0x18u);
}


I2C_Status LCD_I2C_DisplayShiftRight(void)
{
    return LCD_I2C_Command(0x1Cu);
}


//==========================================================
// Caracteres personalizados
//==========================================================

I2C_Status LCD_I2C_CreateChar(
    const uint8_t pattern[8],
    uint8_t position
)
{
    uint8_t index;
    I2C_Status status;


    if ((pattern == NULL) ||
        (position > 7u))
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    /*
     * Cada carácter ocupa ocho direcciones en CGRAM.
     */
    status = LCD_I2C_Command(
        (uint8_t)(
            LCD_CMD_SET_CGRAM |
            (position << 3)
        )
    );


    if (status != I2C_OK)
    {
        return status;
    }


    for (index = 0u; index < 8u; index++)
    {
        status = LCD_I2C_WriteChar(
            (char)pattern[index]
        );


        if (status != I2C_OK)
        {
            return status;
        }
    }


    /*
     * Regresa a la primera posición de DDRAM.
     */
    return LCD_I2C_Command(LCD_CMD_SET_DDRAM);
}


//==========================================================
// Visualización decimal
//==========================================================

static I2C_Status LCD_I2C_WriteFixedUnsigned(
    uint16_t number,
    uint8_t digits,
    uint8_t decimals
)
{
    uint32_t divisor;
    uint8_t index;
    uint8_t digit;
    I2C_Status status;


    if ((digits == 0u) ||
        (digits > 5u) ||
        (decimals >= digits))
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    divisor = 1u;


    /*
     * Calcula el divisor inicial.
     *
     * Para cuatro dígitos:
     *
     * divisor = 1000.
     */
    for (index = 1u; index < digits; index++)
    {
        divisor *= 10u;
    }


    for (index = 0u; index < digits; index++)
    {
        /*
         * Inserta el punto antes de los últimos
         * "decimals" dígitos.
         */
        if ((decimals != 0u) &&
            (index == (uint8_t)(digits - decimals)))
        {
            status = LCD_I2C_WriteChar('.');


            if (status != I2C_OK)
            {
                return status;
            }
        }


        digit = (uint8_t)(
            ((uint32_t)number / divisor) % 10u
        );


        status = LCD_I2C_WriteChar(
            (char)('0' + digit)
        );


        if (status != I2C_OK)
        {
            return status;
        }


        if (divisor > 1u)
        {
            divisor /= 10u;
        }
    }


    return I2C_OK;
}


I2C_Status LCD_I2C_WriteUInt8(
    uint8_t number,
    uint8_t digits
)
{
    if ((digits == 0u) ||
        (digits > 3u))
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    return LCD_I2C_WriteFixedUnsigned(
        number,
        digits,
        0u
    );
}


I2C_Status LCD_I2C_WriteUInt16(
    uint16_t number,
    uint8_t digits,
    uint8_t decimals
)
{
    return LCD_I2C_WriteFixedUnsigned(
        number,
        digits,
        decimals
    );
}


//==========================================================
// Formatos especiales
//==========================================================

I2C_Status LCD_I2C_WriteDegree(void)
{
    return LCD_I2C_WriteChar((char)0xDFu);
}


I2C_Status LCD_I2C_WriteBinary(uint8_t value)
{
    uint8_t index;
    I2C_Status status;


    status = LCD_I2C_WriteString("0b");


    if (status != I2C_OK)
    {
        return status;
    }


    for (index = 0u; index < 8u; index++)
    {
        if (((value >> (7u - index)) & 0x01u) != 0u)
        {
            status = LCD_I2C_WriteChar('1');
        }
        else
        {
            status = LCD_I2C_WriteChar('0');
        }


        if (status != I2C_OK)
        {
            return status;
        }
    }


    return I2C_OK;
}


static char LCD_I2C_NibbleToHex(uint8_t nibble)
{
    nibble &= 0x0Fu;


    if (nibble < 10u)
    {
        return (char)('0' + nibble);
    }


    return (char)('A' + (nibble - 10u));
}


I2C_Status LCD_I2C_WriteHex(uint8_t value)
{
    I2C_Status status;


    status = LCD_I2C_WriteString("0x");


    if (status != I2C_OK)
    {
        return status;
    }


    status = LCD_I2C_WriteChar(
        LCD_I2C_NibbleToHex(
            (uint8_t)(value >> 4)
        )
    );


    if (status != I2C_OK)
    {
        return status;
    }


    return LCD_I2C_WriteChar(
        LCD_I2C_NibbleToHex(value)
    );
}
I2C_Status LCD_I2C_WriteInt(int16_t value)
{
    char buf[8]; // espacio suficiente: signo + 5 dígitos + '\0'
    uint16_t u;
    int pos = 0;

    if (value < 0)
    {
        buf[pos++] = '-';
        u = (uint16_t)(-value);
    }
    else
    {
        u = (uint16_t)value;
    }

    // convertir u a decimal sin sprintf
    if (u == 0)
    {
        buf[pos++] = '0';
    }
    else
    {
        char rev[6];
        int nd = 0;
        while (u > 0 && nd < (int)sizeof(rev))
        {
            rev[nd++] = '0' + (u % 10u);
            u /= 10u;
        }
        while (nd-- > 0)
        {
            buf[pos++] = rev[nd];
        }
    }

    buf[pos] = '\0';
    return LCD_I2C_WriteString(buf);
}
