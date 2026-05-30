# Documentación de la librería `motor_paso`

## Control de motor paso a paso 28BYJ-48 con PIC18F57Q43

Esta documentación describe el funcionamiento, configuración y uso de la librería `motor_paso`, desarrollada para controlar motores paso a paso mediante cuatro señales digitales desde el microcontrolador PIC18F57Q43 usando el compilador XC8 en MPLAB X IDE.

La librería está orientada principalmente al motor paso a paso **28BYJ-48 de 5 V**, utilizado normalmente con el driver **ULN2003**. Sin embargo, también puede emplearse con otros motores paso a paso que trabajen con cuatro señales de control, siempre que se utilice una etapa de potencia adecuada.

---

## Archivos de la librería

La librería está compuesta por dos archivos principales:

```text
motor_paso.h
motor_paso.c
```

### `motor_paso.h`

Contiene:

- Definiciones generales de la librería.
- Constantes de configuración.
- Definición de la estructura `Stepper`.
- Prototipos de las funciones disponibles para el programa principal.

### `motor_paso.c`

Contiene:

- La secuencia de paso completo.
- La escritura interna sobre el puerto seleccionado.
- La implementación de las funciones de inicialización, movimiento, vuelta completa y apagado.

---

## Objetivo de la librería

El objetivo de esta librería es ordenar y simplificar el control de motores paso a paso dentro del proyecto, evitando escribir directamente la secuencia del motor en el archivo principal.

La librería permite:

- Seleccionar el puerto del microcontrolador que controlará el motor.
- Usar la parte baja o alta de un puerto.
- Controlar más de un motor usando estructuras independientes.
- Ejecutar pasos individuales en sentido horario o antihorario.
- Ejecutar una cantidad definida de pasos.
- Ejecutar una vuelta completa del motor 28BYJ-48.
- Apagar las salidas asociadas al motor.

---

## Consideración de hardware

El PIC18F57Q43 no debe alimentar directamente las bobinas del motor paso a paso. Las salidas del microcontrolador solo deben utilizarse como señales de control.

La conexión general debe ser:

```text
PIC18F57Q43 -> Driver ULN2003 -> Motor paso a paso 28BYJ-48
```

Para el uso correcto del sistema, se debe considerar lo siguiente:

- El motor debe alimentarse con una fuente adecuada de 5 V.
- El driver ULN2003 debe recibir las señales desde el PIC.
- La tierra del PIC y la tierra de la fuente del motor deben estar unidas.
- No se debe alimentar el motor directamente desde los pines de salida del PIC.
- Si se usan dos motores, la fuente debe entregar la corriente suficiente para ambos.

---

## Selección de pines mediante nibbles

La librería utiliza cuatro bits de un puerto para controlar un motor. Como un puerto del PIC tiene ocho bits, se puede dividir en dos secciones:

```text
Parte baja del puerto: bits 0, 1, 2 y 3
Parte alta del puerto: bits 4, 5, 6 y 7
```

La selección se realiza mediante las siguientes definiciones:

```c
#define STEPPER_LOW_NIBBLE   0
#define STEPPER_HIGH_NIBBLE  1
```

### `STEPPER_LOW_NIBBLE`

Utiliza los cuatro bits menos significativos del puerto.

Ejemplo usando el puerto D:

```text
RD0, RD1, RD2 y RD3
```

### `STEPPER_HIGH_NIBBLE`

Utiliza los cuatro bits más significativos del puerto.

Ejemplo usando el puerto D:

```text
RD4, RD5, RD6 y RD7
```

Esto permite controlar dos motores usando el mismo puerto:

```text
Motor 1 -> RD0, RD1, RD2, RD3
Motor 2 -> RD4, RD5, RD6, RD7
```

---

## Constantes principales

### `STEPPER_STEPS_PER_REV_28BYJ48`

```c
#define STEPPER_STEPS_PER_REV_28BYJ48  2048
```

Define la cantidad aproximada de pasos necesarios para completar una vuelta del eje externo del motor 28BYJ-48 de 5 V.

Aunque el motor interno tiene menos pasos por vuelta, el 28BYJ-48 incluye una caja reductora interna. Debido a esta reducción mecánica, el eje externo requiere aproximadamente **2048 pasos** para completar una vuelta completa cuando se trabaja con una secuencia de paso completo.

Este valor se utiliza como referencia práctica para aplicaciones académicas y pruebas generales.

### `STEPPER_DEFAULT_DELAY_MS`

```c
#define STEPPER_DEFAULT_DELAY_MS       5
```

Define el retardo por defecto entre pasos, expresado en milisegundos.

Este valor determina la velocidad de giro del motor. Un valor menor aumenta la velocidad, pero si el retardo es demasiado bajo, el motor puede vibrar, perder pasos o no girar correctamente.

Para el motor 28BYJ-48 de 5 V con driver ULN2003, un valor inicial recomendado es **5 ms**.

---

## Estructura `Stepper`

La librería utiliza una estructura para almacenar la configuración de cada motor.

```c
typedef struct
{
    volatile uint8_t *lat;
    volatile uint8_t *tris;
    volatile uint8_t *ansel;
    uint8_t nibble;
    uint8_t index;
} Stepper;
```

Cada motor debe tener su propia variable de tipo `Stepper`.

Ejemplo:

```c
Stepper motor1;
Stepper motor2;
```

### Campos de la estructura

| Campo | Descripción |
|---|---|
| `lat` | Puntero al registro LAT del puerto. Se usa para escribir los niveles lógicos de salida. |
| `tris` | Puntero al registro TRIS del puerto. Se usa para configurar los pines como entrada o salida. |
| `ansel` | Puntero al registro ANSEL del puerto. Se usa para configurar los pines como digitales o analógicos. |
| `nibble` | Indica si el motor usará la parte baja o alta del puerto. |
| `index` | Guarda la posición actual dentro de la secuencia de pasos. |

Esta estructura permite que la misma librería controle varios motores sin que la configuración de uno sobrescriba la del otro.

---

## Secuencia de paso completo

La librería utiliza una secuencia de paso completo con dos fases activas al mismo tiempo.

```c
static const uint8_t full_step_sequence[4] = {
    0b1100,
    0b0110,
    0b0011,
    0b1001
};
```

El orden lógico de las señales es:

```text
IN1 IN2 IN3 IN4
```

La secuencia aplicada es:

| Paso | IN1 | IN2 | IN3 | IN4 | Valor binario |
|---|---:|---:|---:|---:|---|
| 1 | 1 | 1 | 0 | 0 | `1100` |
| 2 | 0 | 1 | 1 | 0 | `0110` |
| 3 | 0 | 0 | 1 | 1 | `0011` |
| 4 | 1 | 0 | 0 | 1 | `1001` |

Esta secuencia proporciona mayor torque que una secuencia de una sola fase activa.

Si el motor vibra pero no gira, o si gira en sentido contrario al esperado, se debe revisar el orden de conexión entre el PIC, el driver ULN2003 y el motor.

---

## Funciones de la librería

## `Stepper_Init()`

```c
void Stepper_Init(Stepper *motor,
                  volatile uint8_t *lat,
                  volatile uint8_t *tris,
                  volatile uint8_t *ansel,
                  uint8_t nibble);
```

Inicializa un motor paso a paso. Esta función guarda en la estructura del motor los registros del puerto seleccionado, configura los cuatro bits elegidos como salidas digitales y apaga las salidas al inicio.

### Parámetros

| Parámetro | Descripción |
|---|---|
| `motor` | Dirección de la estructura `Stepper` asociada al motor. |
| `lat` | Dirección del registro LAT del puerto a utilizar. |
| `tris` | Dirección del registro TRIS del puerto a utilizar. |
| `ansel` | Dirección del registro ANSEL del puerto a utilizar. |
| `nibble` | Selecciona si se usará la parte baja o alta del puerto. |

### Ejemplo

```c
Stepper_Init(&motor1, &LATD, &TRISD, &ANSELD, STEPPER_LOW_NIBBLE);
```

En este caso, el motor `motor1` queda asignado al puerto D, usando los pines:

```text
RD0, RD1, RD2 y RD3
```

---

## `Stepper_Step_CW()`

```c
void Stepper_Step_CW(Stepper *motor);
```

Ejecuta un paso en sentido horario. La función escribe el estado actual de la secuencia en el puerto y luego avanza el índice interno a la siguiente posición.

### Parámetro

| Parámetro | Descripción |
|---|---|
| `motor` | Dirección de la estructura `Stepper` asociada al motor. |

### Ejemplo

```c
Stepper_Step_CW(&motor1);
```

Esta función no incluye un retardo propio entre pasos. El retardo se aplica cuando se usan funciones de movimiento como `Stepper_Move_CW()`.

---

## `Stepper_Step_CCW()`

```c
void Stepper_Step_CCW(Stepper *motor);
```

Ejecuta un paso en sentido antihorario. La función escribe el estado actual de la secuencia en el puerto y luego retrocede el índice interno a la posición anterior.

### Parámetro

| Parámetro | Descripción |
|---|---|
| `motor` | Dirección de la estructura `Stepper` asociada al motor. |

### Ejemplo

```c
Stepper_Step_CCW(&motor1);
```

---

## `Stepper_Move_CW()`

```c
void Stepper_Move_CW(Stepper *motor, uint16_t steps, uint16_t delay_ms);
```

Ejecuta una cantidad determinada de pasos en sentido horario, aplicando un retardo entre cada paso.

### Parámetros

| Parámetro | Descripción |
|---|---|
| `motor` | Dirección de la estructura `Stepper` asociada al motor. |
| `steps` | Número de pasos que debe ejecutar el motor. |
| `delay_ms` | Tiempo de espera entre pasos, expresado en milisegundos. |

### Ejemplo

```c
Stepper_Move_CW(&motor1, 2048, 5);
```

Este ejemplo ejecuta aproximadamente una vuelta completa del eje externo del motor 28BYJ-48 en sentido horario, usando 5 ms de retardo entre pasos.

---

## `Stepper_Move_CCW()`

```c
void Stepper_Move_CCW(Stepper *motor, uint16_t steps, uint16_t delay_ms);
```

Ejecuta una cantidad determinada de pasos en sentido antihorario, aplicando un retardo entre cada paso.

### Parámetros

| Parámetro | Descripción |
|---|---|
| `motor` | Dirección de la estructura `Stepper` asociada al motor. |
| `steps` | Número de pasos que debe ejecutar el motor. |
| `delay_ms` | Tiempo de espera entre pasos, expresado en milisegundos. |

### Ejemplo

```c
Stepper_Move_CCW(&motor1, 2048, 5);
```

Este ejemplo ejecuta aproximadamente una vuelta completa del eje externo del motor 28BYJ-48 en sentido antihorario, usando 5 ms de retardo entre pasos.

---

## `Stepper_fullTurn_CW()`

```c
void Stepper_fullTurn_CW(Stepper *motor);
```

Ejecuta una vuelta completa del eje externo del motor 28BYJ-48 en sentido horario.

Internamente utiliza:

```c
Stepper_Move_CW(motor,
                STEPPER_STEPS_PER_REV_28BYJ48,
                STEPPER_DEFAULT_DELAY_MS);
```

En la versión actual de la librería, después de completar el movimiento, también se llama a:

```c
Stepper_Off(motor);
```

Esto apaga las salidas del motor al finalizar la vuelta completa.

### Ejemplo

```c
Stepper_fullTurn_CW(&motor1);
```

---

## `Stepper_fullTurn_CCW()`

```c
void Stepper_fullTurn_CCW(Stepper *motor);
```

Ejecuta una vuelta completa del eje externo del motor 28BYJ-48 en sentido antihorario.

Internamente utiliza:

```c
Stepper_Move_CCW(motor,
                 STEPPER_STEPS_PER_REV_28BYJ48,
                 STEPPER_DEFAULT_DELAY_MS);
```

En la versión actual de la librería, después de completar el movimiento, también se llama a:

```c
Stepper_Off(motor);
```

Esto apaga las salidas del motor al finalizar la vuelta completa.

### Ejemplo

```c
Stepper_fullTurn_CCW(&motor1);
```

---

## `Stepper_Off()`

```c
void Stepper_Off(Stepper *motor);
```

Desactiva las cuatro salidas asignadas al motor, colocando en cero los bits correspondientes del puerto.

### Parámetro

| Parámetro | Descripción |
|---|---|
| `motor` | Dirección de la estructura `Stepper` asociada al motor. |

### Ejemplo

```c
Stepper_Off(&motor1);
```

### Consideración

Al apagar las salidas, el motor pierde torque de retención. Si existe una carga mecánica aplicada al eje, este podría moverse al quedar desenergizado.

---

## Ejemplo de configuración para un motor

Este ejemplo configura un motor 28BYJ-48 usando el puerto D, parte baja.

```c
#include "cabecera.h"
#include "motor_paso.h"

Stepper motor1;

void main(void)
{
    config();

    Stepper_Init(&motor1, &LATD, &TRISD, &ANSELD, STEPPER_LOW_NIBBLE);

    while(1)
    {
        Stepper_fullTurn_CW(&motor1);
        __delay_ms(1000);

        Stepper_fullTurn_CCW(&motor1);
        __delay_ms(1000);
    }
}
```

### Conexión física

```text
IN1 -> RD0
IN2 -> RD1
IN3 -> RD2
IN4 -> RD3
```

---

## Ejemplo de configuración para dos motores en el mismo puerto

Este ejemplo configura dos motores en el mismo puerto D.

```c
#include "cabecera.h"
#include "motor_paso.h"

Stepper motor1;
Stepper motor2;

void main(void)
{
    config();

    Stepper_Init(&motor1, &LATD, &TRISD, &ANSELD, STEPPER_LOW_NIBBLE);
    Stepper_Init(&motor2, &LATD, &TRISD, &ANSELD, STEPPER_HIGH_NIBBLE);

    while(1)
    {
        Stepper_fullTurn_CW(&motor1);
        __delay_ms(1000);

        Stepper_fullTurn_CW(&motor2);
        __delay_ms(1000);

        Stepper_fullTurn_CCW(&motor1);
        __delay_ms(1000);

        Stepper_fullTurn_CCW(&motor2);
        __delay_ms(1000);
    }
}
```

### Conexión física

```text
Motor 1 con ULN2003:
IN1 -> RD0
IN2 -> RD1
IN3 -> RD2
IN4 -> RD3

Motor 2 con ULN2003:
IN1 -> RD4
IN2 -> RD5
IN3 -> RD6
IN4 -> RD7
```

---

## Dependencia con `cabecera.h`

La librería incluye el archivo:

```c
#include "cabecera.h"
```

Este archivo debe contener, como mínimo:

```c
#include <xc.h>
#include <stdint.h>
#define _XTAL_FREQ 48000000UL
```

La constante `_XTAL_FREQ` es necesaria porque la librería utiliza `__delay_ms(1)` dentro de las funciones `Stepper_Move_CW()` y `Stepper_Move_CCW()`.

Si se cambia la frecuencia del sistema, solo debe modificarse la definición de `_XTAL_FREQ` en `cabecera.h`.

---

## Recomendaciones de uso

- Usar una fuente externa de 5 V para alimentar los motores.
- Unir la tierra de la fuente externa con la tierra del PIC.
- No alimentar las bobinas del motor directamente desde el microcontrolador.
- Iniciar las pruebas con un retardo de 5 ms entre pasos.
- Si el motor vibra o pierde pasos, aumentar el retardo a 8 ms o 10 ms.
- Si el motor gira en sentido contrario al esperado, intercambiar el orden lógico de las señales o usar la función de giro opuesta.
- Si el motor no gira y solo vibra, revisar el orden de conexión entre IN1, IN2, IN3 e IN4.

---

## Resumen de funciones

| Función | Descripción |
|---|---|
| `Stepper_Init()` | Inicializa un motor y configura sus salidas digitales. |
| `Stepper_Step_CW()` | Ejecuta un paso en sentido horario. |
| `Stepper_Step_CCW()` | Ejecuta un paso en sentido antihorario. |
| `Stepper_Move_CW()` | Ejecuta varios pasos en sentido horario con retardo entre pasos. |
| `Stepper_Move_CCW()` | Ejecuta varios pasos en sentido antihorario con retardo entre pasos. |
| `Stepper_fullTurn_CW()` | Ejecuta una vuelta completa en sentido horario usando valores por defecto. |
| `Stepper_fullTurn_CCW()` | Ejecuta una vuelta completa en sentido antihorario usando valores por defecto. |
| `Stepper_Off()` | Apaga las cuatro salidas asignadas al motor. |

---

## Limitaciones de la librería

- La librería usa retardos bloqueantes mediante `__delay_ms()`. Mientras el motor se mueve, el programa permanece ocupado ejecutando el movimiento.
- La precisión del giro depende del motor, del driver, de la fuente de alimentación y de la carga mecánica.
- El valor de 2048 pasos por vuelta es una referencia práctica para el 28BYJ-48, no una medición exacta absoluta.
- La librería está pensada para control secuencial. No implementa movimiento simultáneo real de varios motores.

---

## Observación final

La librería `motor_paso` permite separar la lógica de control del motor del programa principal. Esto mejora el orden del código, facilita la reutilización y permite controlar más de un motor sin duplicar la lógica de movimiento.

Para el proyecto del dispensador automático, esta organización permite asignar cada mecanismo de dispensación a una estructura `Stepper`, manteniendo un código más claro y modular.
