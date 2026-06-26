#ifndef LCD_I2C_H
#define LCD_I2C_H

/**
 * @file LCD_I2C.h
 * @brief Control de LCD HD44780 mediante PCF8574/PCF8574A.
 *
 * Esta librería contiene únicamente elementos propios del LCD:
 *
 * - Dirección del backpack.
 * - Señales RS, RW y Enable.
 * - Backlight.
 * - Comandos del HD44780.
 * - Posicionamiento del cursor.
 * - Escritura de textos y variables.
 *
 * La transmisión física se realiza mediante la librería I2C.
 */

#include <stdint.h>

#include "I2C.h"


//==========================================================
// Estados de encendido y apagado
//==========================================================

#define LCD_I2C_ON   1u
#define LCD_I2C_OFF  0u


//==========================================================
// Direcciones del PCF8574
//==========================================================

#define PCF8574_0   0x20u
#define PCF8574_1   0x21u
#define PCF8574_2   0x22u
#define PCF8574_3   0x23u
#define PCF8574_4   0x24u
#define PCF8574_5   0x25u
#define PCF8574_6   0x26u
#define PCF8574_7   0x27u


//==========================================================
// Direcciones del PCF8574A
//==========================================================

#define PCF8574A_0  0x38u
#define PCF8574A_1  0x39u
#define PCF8574A_2  0x3Au
#define PCF8574A_3  0x3Bu
#define PCF8574A_4  0x3Cu
#define PCF8574A_5  0x3Du
#define PCF8574A_6  0x3Eu
#define PCF8574A_7  0x3Fu


//==========================================================
// Retardos proporcionados por main.c
//==========================================================

/**
 * Estas funciones deben implementarse en el programa principal.
 *
 * De esta manera, LCD_I2C.c no necesita definir _XTAL_FREQ.
 */
void LCD_I2C_DelayUs(uint16_t timeUs);
void LCD_I2C_DelayMs(uint16_t timeMs);


//==========================================================
// Configuración
//==========================================================

/**
 * @brief Selecciona la dirección de 7 bits del backpack.
 */
I2C_Status LCD_I2C_SetAddress(uint8_t address7);


/**
 * @brief Devuelve la dirección actualmente seleccionada.
 */
uint8_t LCD_I2C_GetAddress(void);


/**
 * @brief Inicializa solamente el controlador LCD.
 *
 * I2C1 debe inicializarse previamente desde el programa principal.
 */
I2C_Status LCD_I2C_Init(void);


//==========================================================
// Envío de comandos y datos
//==========================================================

I2C_Status LCD_I2C_Command(uint8_t command);
I2C_Status LCD_I2C_WriteChar(char character);

I2C_Status LCD_I2C_WriteString(const char *text);

I2C_Status LCD_I2C_WriteStringN(
    const char *text,
    uint8_t length
);


//==========================================================
// Posición y control del LCD
//==========================================================

/**
 * @brief Posiciona el cursor.
 *
 * La fila comienza en 1 y la columna comienza en 0.
 */
I2C_Status LCD_I2C_SetCursor(
    uint8_t row,
    uint8_t column
);

I2C_Status LCD_I2C_Clear(void);
I2C_Status LCD_I2C_Home(void);

I2C_Status LCD_I2C_Display(uint8_t state);
I2C_Status LCD_I2C_Cursor(uint8_t state);
I2C_Status LCD_I2C_Blink(uint8_t state);
I2C_Status LCD_I2C_Backlight(uint8_t state);

I2C_Status LCD_I2C_CursorShiftLeft(void);
I2C_Status LCD_I2C_CursorShiftRight(void);
I2C_Status LCD_I2C_DisplayShiftLeft(void);
I2C_Status LCD_I2C_DisplayShiftRight(void);


//==========================================================
// Caracteres personalizados
//==========================================================

/**
 * @brief Crea un carácter personalizado.
 *
 * @param pattern Patrón de ocho filas.
 * @param position Posición de CGRAM entre 0 y 7.
 */
I2C_Status LCD_I2C_CreateChar(
    const uint8_t pattern[8],
    uint8_t position
);


//==========================================================
// Visualización de variables
//==========================================================

I2C_Status LCD_I2C_WriteUInt8(
    uint8_t number,
    uint8_t digits
);


/**
 * @brief Escribe un entero con formato decimal fijo.
 *
 * decimals indica cuántos dígitos se colocan después del punto.
 *
 * Ejemplo:
 *
 * number   = 1234
 * digits   = 4
 * decimals = 2
 *
 * Resultado:
 *
 * 12.34
 */
I2C_Status LCD_I2C_WriteUInt16(
    uint16_t number,
    uint8_t digits,
    uint8_t decimals
);

I2C_Status LCD_I2C_WriteDegree(void);
I2C_Status LCD_I2C_WriteBinary(uint8_t value);
I2C_Status LCD_I2C_WriteHex(uint8_t value);


#endif /* LCD_I2C_H */


