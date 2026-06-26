# Librería LCD_I2C para LCD HD44780 con PCF8574

## 1. Descripción

La librería `LCD_I2C` permite controlar una pantalla LCD compatible con el controlador **HD44780** mediante un módulo expansor I2C **PCF8574** o **PCF8574A**.

![LCD_I2C - Imagen](../Docs/img/LCD_I2C.jfif)

La librería utiliza la capa genérica `I2C` para realizar todas las transmisiones. No configura directamente el periférico I2C del microcontrolador.

Funciones principales:

- Inicialización del LCD en modo de 4 bits.
- Selección de la dirección I2C del backpack.
- Escritura de caracteres y cadenas.
- Posicionamiento del cursor.
- Control del display, cursor y parpadeo.
- Control de la iluminación de fondo.
- Desplazamiento de cursor y pantalla.
- Creación de caracteres personalizados.
- Escritura de valores decimales, binarios y hexadecimales.

---

## 2. Archivos

```text
LCD_I2C.h
LCD_I2C.c
```

### Dependencias

```text
I2C.h
I2C.c
cabecera.h
```

`cabecera.h` debe contener:

```c
#define _XTAL_FREQ 32000000UL
```

El valor debe coincidir con la frecuencia real configurada en el microcontrolador, porque `LCD_I2C.c` utiliza:

```c
__delay_ms();
__delay_us();
```

---

## 3. Hardware compatible

- LCD 16×2.
- LCD 16×4.
- LCD 20×4 compatible con HD44780.
- Backpack basado en PCF8574.
- Backpack basado en PCF8574A.

La librería asume el siguiente mapeo típico:

| PCF8574 | LCD |
|---|---|
| P0 | RS |
| P1 | RW |
| P2 | E |
| P3 | Backlight |
| P4 | D4 |
| P5 | D5 |
| P6 | D6 |
| P7 | D7 |

La librería trabaja solamente en escritura, por lo que `RW` permanece en cero.

---

## 4. Conexiones

```text
PIC18F57Q43             LCD I2C
-----------------------------------------
RC3 / SCL      -------> SCL
RC4 / SDA      <------> SDA
5 V            -------> VCC
GND            -------> GND
```

El LCD y otros dispositivos pueden compartir el mismo bus I2C.

---

## 5. Direcciones disponibles

### PCF8574

```c
PCF8574_0   /* 0x20 */
PCF8574_1   /* 0x21 */
PCF8574_2   /* 0x22 */
PCF8574_3   /* 0x23 */
PCF8574_4   /* 0x24 */
PCF8574_5   /* 0x25 */
PCF8574_6   /* 0x26 */
PCF8574_7   /* 0x27 */
```

La dirección más común es:

```c
PCF8574_7
```

### PCF8574A

```c
PCF8574A_0  /* 0x38 */
PCF8574A_1  /* 0x39 */
PCF8574A_2  /* 0x3A */
PCF8574A_3  /* 0x3B */
PCF8574A_4  /* 0x3C */
PCF8574A_5  /* 0x3D */
PCF8574A_6  /* 0x3E */
PCF8574A_7  /* 0x3F */
```

Una dirección frecuente para este modelo es:

```c
PCF8574A_7
```

---

## 6. Inicialización

El orden correcto es:

```c
I2C1_Init();

LCD_I2C_SetAddress(PCF8574_7);

LCD_I2C_Init();
```

`I2C1_Init()` debe ejecutarse una sola vez.

Ejemplo:

```c
I2C_Status estadoLCD;

I2C1_Init();

estadoLCD = LCD_I2C_SetAddress(
    PCF8574_7
);

if (estadoLCD == I2C_OK)
{
    estadoLCD = LCD_I2C_Init();
}
```

---

## 7. Estados de encendido y apagado

La librería utiliza:

```c
#define LCD_I2C_ON   1u
#define LCD_I2C_OFF  0u
```

Ejemplo:

```c
LCD_I2C_Backlight(LCD_I2C_ON);
LCD_I2C_Cursor(LCD_I2C_OFF);
```

---

## 8. Funciones públicas

### 8.1 `LCD_I2C_SetAddress`

```c
I2C_Status LCD_I2C_SetAddress(uint8_t address7);
```

Selecciona la dirección de 7 bits del backpack.

Ejemplo:

```c
LCD_I2C_SetAddress(PCF8574_7);
```

La dirección debe seleccionarse antes de llamar a `LCD_I2C_Init()`.

---

### 8.2 `LCD_I2C_GetAddress`

```c
uint8_t LCD_I2C_GetAddress(void);
```

Devuelve la dirección actualmente configurada.

Ejemplo:

```c
uint8_t direccion;

direccion = LCD_I2C_GetAddress();
```

---

### 8.3 `LCD_I2C_Init`

```c
I2C_Status LCD_I2C_Init(void);
```

Inicializa el HD44780 en:

- Modo de 4 bits.
- Dos líneas lógicas.
- Caracteres de 5×8 puntos.
- Incremento automático del cursor.
- Display encendido.
- Cursor apagado.
- Parpadeo apagado.
- Backlight encendido.

Antes de llamarla se debe inicializar el bus:

```c
I2C1_Init();
```

---

### 8.4 `LCD_I2C_Command`

```c
I2C_Status LCD_I2C_Command(uint8_t command);
```

Envía un comando directamente al controlador HD44780.

Ejemplo:

```c
LCD_I2C_Command(0x01u);
```

Para operaciones comunes se recomienda utilizar las funciones específicas de la librería.

---

### 8.5 `LCD_I2C_WriteChar`

```c
I2C_Status LCD_I2C_WriteChar(char character);
```

Escribe un carácter.

Ejemplo:

```c
LCD_I2C_WriteChar('A');
```

---

### 8.6 `LCD_I2C_WriteString`

```c
I2C_Status LCD_I2C_WriteString(const char *text);
```

Escribe una cadena terminada en `'\0'`.

Ejemplo:

```c
LCD_I2C_WriteString("Hola mundo");
```

---

### 8.7 `LCD_I2C_WriteStringN`

```c
I2C_Status LCD_I2C_WriteStringN(
    const char *text,
    uint8_t length
);
```

Escribe exactamente la cantidad indicada de caracteres.

Ejemplo:

```c
LCD_I2C_WriteStringN(
    "ABCDEF",
    3u
);
```

Resultado:

```text
ABC
```

---

### 8.8 `LCD_I2C_SetCursor`

```c
I2C_Status LCD_I2C_SetCursor(
    uint8_t row,
    uint8_t column
);
```

Posiciona el cursor.

- La fila comienza en `1`.
- La columna comienza en `0`.

Ejemplo:

```c
LCD_I2C_SetCursor(2u, 0u);
LCD_I2C_WriteString("Segunda fila");
```

Direcciones de fila utilizadas:

| Fila | Dirección DDRAM inicial |
|---:|---:|
| 1 | `0x00` |
| 2 | `0x40` |
| 3 | `0x14` |
| 4 | `0x54` |

---

### 8.9 `LCD_I2C_Clear`

```c
I2C_Status LCD_I2C_Clear(void);
```

Borra el contenido visible y coloca el cursor al inicio.

Ejemplo:

```c
LCD_I2C_Clear();
```

Este comando necesita aproximadamente 2 ms para completarse.

No se recomienda ejecutarlo continuamente dentro de un ciclo, porque puede producir parpadeo.

---

### 8.10 `LCD_I2C_Home`

```c
I2C_Status LCD_I2C_Home(void);
```

Regresa el cursor a la posición inicial sin borrar el contenido.

---

### 8.11 `LCD_I2C_Display`

```c
I2C_Status LCD_I2C_Display(uint8_t state);
```

Enciende o apaga la visualización.

```c
LCD_I2C_Display(LCD_I2C_ON);
LCD_I2C_Display(LCD_I2C_OFF);
```

Apagar el display no elimina los datos de DDRAM.

---

### 8.12 `LCD_I2C_Cursor`

```c
I2C_Status LCD_I2C_Cursor(uint8_t state);
```

Controla la visibilidad del cursor.

```c
LCD_I2C_Cursor(LCD_I2C_ON);
```

---

### 8.13 `LCD_I2C_Blink`

```c
I2C_Status LCD_I2C_Blink(uint8_t state);
```

Activa o desactiva el parpadeo del cursor.

```c
LCD_I2C_Blink(LCD_I2C_ON);
```

---

### 8.14 `LCD_I2C_Backlight`

```c
I2C_Status LCD_I2C_Backlight(uint8_t state);
```

Controla la iluminación de fondo.

```c
LCD_I2C_Backlight(LCD_I2C_ON);
LCD_I2C_Backlight(LCD_I2C_OFF);
```

Esta función depende del mapeo del backpack y de que el jumper del backlight esté instalado.

---

### 8.15 Desplazamiento del cursor

```c
LCD_I2C_CursorShiftLeft();
LCD_I2C_CursorShiftRight();
```

Mueven el cursor sin alterar el texto.

---

### 8.16 Desplazamiento del display

```c
LCD_I2C_DisplayShiftLeft();
LCD_I2C_DisplayShiftRight();
```

Desplazan todo el contenido visible.

---

### 8.17 `LCD_I2C_CreateChar`

```c
I2C_Status LCD_I2C_CreateChar(
    const uint8_t pattern[8],
    uint8_t position
);
```

Crea un carácter personalizado en una posición de CGRAM entre 0 y 7.

Ejemplo:

```c
const uint8_t corazon[8] =
{
    0b00000,
    0b01010,
    0b11111,
    0b11111,
    0b01110,
    0b00100,
    0b00000,
    0b00000
};

LCD_I2C_CreateChar(corazon, 0u);

LCD_I2C_SetCursor(1u, 0u);
LCD_I2C_WriteChar((char)0u);
```

Cada fila utiliza solamente los cinco bits menos significativos.

---

### 8.18 `LCD_I2C_WriteUInt8`

```c
I2C_Status LCD_I2C_WriteUInt8(
    uint8_t number,
    uint8_t digits
);
```

Escribe un valor de 8 bits con una cantidad fija de dígitos.

Ejemplo:

```c
LCD_I2C_WriteUInt8(5u, 3u);
```

Resultado:

```text
005
```

---

### 8.19 `LCD_I2C_WriteUInt16`

```c
I2C_Status LCD_I2C_WriteUInt16(
    uint16_t number,
    uint8_t digits,
    uint8_t decimals
);
```

Escribe un valor con formato decimal fijo.

Ejemplo:

```c
LCD_I2C_WriteUInt16(
    1234u,
    4u,
    2u
);
```

Resultado:

```text
12.34
```

`digits` corresponde al número de dígitos numéricos y `decimals` indica cuántos aparecen después del punto.

---

### 8.20 `LCD_I2C_WriteDegree`

```c
I2C_Status LCD_I2C_WriteDegree(void);
```

Escribe el símbolo de grado:

```c
LCD_I2C_WriteString("25");
LCD_I2C_WriteDegree();
LCD_I2C_WriteChar('C');
```

Resultado:

```text
25°C
```

---

### 8.21 `LCD_I2C_WriteBinary`

```c
I2C_Status LCD_I2C_WriteBinary(uint8_t value);
```

Escribe un byte en formato binario.

Ejemplo:

```c
LCD_I2C_WriteBinary(0xA5u);
```

Resultado:

```text
0b10100101
```

---

### 8.22 `LCD_I2C_WriteHex`

```c
I2C_Status LCD_I2C_WriteHex(uint8_t value);
```

Escribe un byte en formato hexadecimal.

Ejemplo:

```c
LCD_I2C_WriteHex(0xA5u);
```

Resultado:

```text
0xA5
```

---

## 9. Ejemplo básico

```c
#include <xc.h>
#include <stdint.h>

#include "cabecera.h"
#include "I2C.h"
#include "LCD_I2C.h"

void main(void)
{
    I2C_Status estado;

    I2C1_Init();

    estado = LCD_I2C_SetAddress(
        PCF8574_7
    );

    if (estado == I2C_OK)
    {
        estado = LCD_I2C_Init();
    }

    if (estado != I2C_OK)
    {
        while (1)
        {
        }
    }

    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1u, 0u);
    LCD_I2C_WriteString("PIC18F57Q43");

    LCD_I2C_SetCursor(2u, 0u);
    LCD_I2C_WriteString("LCD I2C activo");

    while (1)
    {
    }
}
```

---

## 10. Actualización sin parpadeo

No se recomienda:

```c
while (1)
{
    LCD_I2C_Clear();
    LCD_I2C_WriteString("Hora...");
}
```

Es preferible sobrescribir las posiciones existentes:

```c
while (1)
{
    LCD_I2C_SetCursor(1u, 0u);
    LCD_I2C_WriteString("Hora 12:34:56");
}
```

Se pueden agregar espacios al final para eliminar caracteres anteriores:

```c
LCD_I2C_WriteString("RTC desconectado ");
```

---

## 11. Manejo de errores

Todas las funciones principales devuelven `I2C_Status`.

Ejemplo:

```c
I2C_Status estado;

estado = LCD_I2C_WriteString("Prueba");

if (estado != I2C_OK)
{
    /*
     * Revisar dirección, alimentación,
     * cableado y estado del bus.
     */
}
```

Un `I2C_ERROR_NACK` suele indicar:

- Dirección incorrecta.
- LCD desconectado.
- Backpack sin alimentación.
- SDA o SCL mal conectados.

---

## 12. Problemas comunes

### El backlight no enciende

Revisar:

- VCC y GND.
- Jumper del backlight.
- `LCD_I2C_Backlight(LCD_I2C_ON)`.
- Mapeo del backpack.

### El backlight enciende, pero no aparece texto

Revisar:

- Potenciómetro de contraste.
- Dirección `0x27` o `0x3F`.
- Inicialización previa de I2C1.
- Frecuencia real de `_XTAL_FREQ`.
- Mapeo PCF8574–HD44780.

### Aparecen cuadrados oscuros

El LCD tiene alimentación, pero probablemente no fue inicializado correctamente.

### El texto se desplaza a posiciones incorrectas

Verificar el tamaño real del LCD y las direcciones de fila.

---

## 13. Consideraciones sobre `_XTAL_FREQ`

En `cabecera.h`:

```c
#ifndef CABECERA_H
#define CABECERA_H

#include <xc.h>

#define _XTAL_FREQ 32000000UL

#endif
```

El valor debe coincidir con el oscilador:

```c
OSCCON1 = 0x60u;
OSCFRQ  = 0x06u;
OSCEN   = 0x40u;
```

Si el microcontrolador trabaja a otra frecuencia, `_XTAL_FREQ` debe modificarse.

---

## 14. Limitaciones

- La librería no lee el Busy Flag.
- `RW` permanece en cero.
- Los tiempos de ejecución se respetan mediante retardos.
- El mapeo del backpack debe coincidir con el definido.
- Solo se permite una dirección activa a la vez.
- No existe protección para llamadas concurrentes.
- Las funciones de impresión numérica no manejan signo negativo.
- La función de formato fijo no realiza redondeo.

---

## 15. Organización recomendada

```text
Header Files
├── cabecera.h
├── I2C.h
└── LCD_I2C.h

Source Files
├── main.c
├── I2C.c
└── LCD_I2C.c
```
