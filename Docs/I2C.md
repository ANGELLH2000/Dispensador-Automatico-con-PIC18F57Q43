# Librería I2C para PIC18F57Q43

## 1. Descripción

La librería `I2C` proporciona una capa de comunicación genérica para el periférico **I2C1** del microcontrolador **PIC18F57Q43**, utilizando el compilador **MPLAB XC8**.

Su función es encargarse únicamente del bus I2C:

- Configuración del periférico I2C1.
- Configuración de los pines mediante PPS.
- Generación de transacciones de lectura y escritura.
- Manejo de direcciones de 7 bits.
- Detección de ACK y NACK.
- Detección de colisiones y tiempos de espera.
- Recuperación del módulo cuando un dispositivo no responde.

La librería no contiene código específico para dispositivos como LCD, RTC, sensores o memorias. Estos dispositivos deben utilizar la librería como una capa inferior de comunicación.

---

## 2. Archivos

La librería está formada por:

```text
I2C.h
I2C.c
```

### `I2C.h`

Contiene:

- Enumeración de estados.
- Declaración de la variable de estado global.
- Prototipos de las funciones públicas.

### `I2C.c`

Contiene:

- Configuración del periférico I2C1.
- Configuración PPS.
- Funciones privadas de espera y recuperación.
- Implementación de lectura y escritura.

---

## 3. Requisitos

- Microcontrolador: `PIC18F57Q43`.
- Compilador: `MPLAB XC8`.
- Estándar utilizado: C99.
- Pines:
  - `RC3` → SCL.
  - `RC4` → SDA.
- Resistencias pull-up externas en SCL y SDA.
- Dispositivos compatibles con I2C estándar de 100 kHz.

La librería configura el bus a **100 kHz** utilizando `MFINTOSC = 500 kHz` como fuente del periférico I2C:

```text
fSCL = 500 kHz / 5 = 100 kHz
```

Por esta razón, la velocidad del bus I2C no depende directamente de `_XTAL_FREQ`.

---

## 4. Conexiones

```text
PIC18F57Q43              Dispositivo I2C
------------------------------------------------
RC3 / SCL      --------> SCL
RC4 / SDA      <-------> SDA
GND            --------> GND
VCC            --------> VCC
```

Las líneas SCL y SDA requieren resistencias pull-up. Muchos módulos ya incluyen estas resistencias.

Una configuración típica es:

```text
VCC
 |
4.7 kΩ
 |
SCL
```

y otra resistencia igual para SDA.

---

## 5. Inclusión de la librería

En el programa principal:

```c
#include "I2C.h"
```

Antes de utilizar cualquier dispositivo I2C:

```c
I2C1_Init();
```

El bus debe inicializarse una sola vez, aunque se conecten varios dispositivos.

---

## 6. Estados de operación

La librería utiliza el tipo:

```c
typedef enum
{
    I2C_OK = 0,
    I2C_ERROR_NACK,
    I2C_ERROR_COLLISION,
    I2C_ERROR_BUS_TIMEOUT,
    I2C_ERROR_SOFTWARE_TIMEOUT,
    I2C_ERROR_INVALID_PARAMETER
} I2C_Status;
```

| Estado | Descripción |
|---|---|
| `I2C_OK` | La operación terminó correctamente. |
| `I2C_ERROR_NACK` | El dispositivo no respondió o rechazó un byte. |
| `I2C_ERROR_COLLISION` | Se detectó una colisión en el bus. |
| `I2C_ERROR_BUS_TIMEOUT` | Se produjo un timeout del periférico. |
| `I2C_ERROR_SOFTWARE_TIMEOUT` | Se agotó el contador de seguridad por software. |
| `I2C_ERROR_INVALID_PARAMETER` | Se recibió un puntero, longitud o dirección inválida. |

La variable:

```c
extern volatile I2C_Status I2C1_LastStatus;
```

conserva el resultado de la operación más reciente.

Ejemplo:

```c
I2C_Status estado;

estado = I2C1_WriteSingleByte(0x68u, 0x00u);

if (estado != I2C_OK)
{
    /* Manejo del error. */
}
```

---

## 7. Direcciones I2C

Las funciones reciben direcciones de **7 bits**.

Ejemplo para el DS1307:

```text
Dirección de 7 bits: 0x68
Escritura en el bus: 0xD0
Lectura en el bus:   0xD1
```

La aplicación debe usar:

```c
#define DS1307_ADDRESS 0x68u
```

No debe pasar directamente `0xD0` ni `0xD1`, porque la librería añade internamente el bit `R/W`.

---

## 8. Funciones públicas

### 8.1 `I2C1_Init`

```c
void I2C1_Init(void);
```

Inicializa I2C1 como maestro con direccionamiento de 7 bits y velocidad de 100 kHz.

Configuración aplicada:

- `RC3` como SCL.
- `RC4` como SDA.
- Pines digitales.
- Salidas open-drain.
- PPS de entrada y salida.
- Interrupciones I2C deshabilitadas.
- Funcionamiento mediante polling.

Ejemplo:

```c
void main(void)
{
    I2C1_Init();

    while (1)
    {
    }
}
```

---

### 8.2 `I2C1_Write`

```c
I2C_Status I2C1_Write(
    uint8_t address7,
    const uint8_t *data,
    uint8_t length
);
```

Envía uno o varios bytes dentro de una única transacción.

#### Parámetros

| Parámetro | Descripción |
|---|---|
| `address7` | Dirección I2C de 7 bits. |
| `data` | Arreglo con los bytes que serán enviados. |
| `length` | Cantidad de bytes que serán transmitidos. |

#### Ejemplo

```c
uint8_t datos[2];

datos[0] = 0x00u;
datos[1] = 0x25u;

I2C_Status estado;

estado = I2C1_Write(
    0x68u,
    datos,
    2u
);
```

La transacción generada será:

```text
START
Dirección + escritura
0x00
0x25
STOP
```

---

### 8.3 `I2C1_WriteSingleByte`

```c
I2C_Status I2C1_WriteSingleByte(
    uint8_t address7,
    uint8_t data
);
```

Envía un solo byte a un dispositivo.

Ejemplo:

```c
I2C_Status estado;

estado = I2C1_WriteSingleByte(
    0x68u,
    0x00u
);
```

Esta función utiliza internamente `I2C1_Write()` con una longitud de un byte.

---

### 8.4 `I2C1_Read`

```c
I2C_Status I2C1_Read(
    uint8_t address7,
    uint8_t *data,
    uint8_t length
);
```

Lee uno o varios bytes consecutivos desde un dispositivo I2C.

#### Parámetros

| Parámetro | Descripción |
|---|---|
| `address7` | Dirección I2C de 7 bits. |
| `data` | Arreglo donde se almacenarán los bytes recibidos. |
| `length` | Cantidad de bytes que deben leerse. |

#### Ejemplo

```c
uint8_t datos[7];
I2C_Status estado;

estado = I2C1_Read(
    0x68u,
    datos,
    7u
);
```

Al finalizar el último byte, el maestro genera un NACK y luego una condición STOP.

---

## 9. Selección de registros antes de una lectura

Muchos dispositivos utilizan un puntero interno de registros. En esos casos se debe:

1. Escribir la dirección del registro.
2. Leer los datos.

Ejemplo con el DS1307:

```c
uint8_t registroInicial = 0x00u;
uint8_t datos[7];
I2C_Status estado;

estado = I2C1_WriteSingleByte(
    0x68u,
    registroInicial
);

if (estado == I2C_OK)
{
    estado = I2C1_Read(
        0x68u,
        datos,
        7u
    );
}
```

La primera escritura solamente posiciona el puntero interno del dispositivo.

---

## 10. Uso con varios dispositivos

Todos los dispositivos pueden compartir SCL y SDA mientras tengan direcciones diferentes.

Ejemplo:

```text
LCD PCF8574: 0x27
DS1307:      0x68
```

Conexión:

```text
RC3 / SCL ----+---- LCD SCL
              |
              +---- DS1307 SCL

RC4 / SDA ----+---- LCD SDA
              |
              +---- DS1307 SDA
```

`I2C1_Init()` debe ejecutarse una sola vez.

---

## 11. Recuperación ante errores

Cuando un dispositivo no está conectado, normalmente se produce:

```c
I2C_ERROR_NACK
```

La implementación reinicia la máquina de estados del periférico ante errores, permitiendo que otros dispositivos del bus continúen funcionando.

Ejemplo:

```c
I2C_Status estadoRTC;

estadoRTC = I2C1_WriteSingleByte(
    0x68u,
    0x00u
);

if (estadoRTC != I2C_OK)
{
    /*
     * El programa puede continuar usando el LCD
     * u otros dispositivos del mismo bus.
     */
}
```

La aplicación no debe detenerse necesariamente ante un NACK si el dispositivo es opcional o puede estar desconectado.

---

## 12. Ejemplo básico completo

```c
#include <xc.h>
#include <stdint.h>

#include "cabecera.h"
#include "I2C.h"

#define DEVICE_ADDRESS 0x68u

void main(void)
{
    uint8_t registro = 0x00u;
    uint8_t datos[7];
    I2C_Status estado;

    I2C1_Init();

    estado = I2C1_WriteSingleByte(
        DEVICE_ADDRESS,
        registro
    );

    if (estado == I2C_OK)
    {
        estado = I2C1_Read(
            DEVICE_ADDRESS,
            datos,
            7u
        );
    }

    while (1)
    {
    }
}
```

---

## 13. Recomendaciones

- Utilizar siempre direcciones de 7 bits.
- Comprobar el valor retornado por cada función.
- Inicializar I2C1 una sola vez.
- Mantener SCL y SDA con resistencias pull-up.
- No utilizar una velocidad mayor a la admitida por el dispositivo.
- Conectar todos los módulos a una referencia de tierra común.
- Realizar `Clean and Build Project` después de modificar los archivos de la librería.
- Evitar llamadas simultáneas desde interrupciones y desde el programa principal sin un mecanismo de exclusión.

---

## 14. Limitaciones

- Funciona mediante polling.
- No utiliza interrupciones.
- Está configurada específicamente para `I2C1`.
- Utiliza `RC3` y `RC4`.
- Trabaja a 100 kHz.
- No implementa arbitraje para múltiples maestros.
- No es segura para llamadas concurrentes.
- La longitud está limitada a `uint8_t`, es decir, un máximo teórico de 255 bytes por operación.

---

## 15. Organización recomendada

```text
Header Files
└── I2C.h

Source Files
└── I2C.c
```

En un proyecto con LCD y RTC:

```text
Header Files
├── cabecera.h
├── I2C.h
├── LCD_I2C.h
└── DS1307.h

Source Files
├── main.c
├── I2C.c
├── LCD_I2C.c
└── DS1307.c
```
