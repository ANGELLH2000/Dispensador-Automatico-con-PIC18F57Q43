#ifndef MOTOR_PASO_H
#define MOTOR_PASO_H

#include "cabecera.h"

/*
 * Librería para control de motores paso a paso mediante 4 señales digitales.
 *
 * Esta librería permite controlar motores paso a paso de 4 entradas, como
 * el motor 28BYJ-48 de 5 V conectado mediante un driver ULN2003.
 *
 * La librería permite seleccionar:
 *
 * - El puerto del microcontrolador que se utilizará.
 * - La sección del puerto asignada al motor.
 * - La cantidad de pasos a ejecutar.
 * - El retardo entre pasos.
 *
 * A diferencia de una librería básica para un solo motor, esta versión permite
 * controlar varios motores, ya que cada motor posee su propia estructura
 * de configuración.
 *
 * El microcontrolador no debe alimentar directamente las bobinas del motor.
 * Las salidas del PIC18F57Q43 deben conectarse a un driver de potencia.
 */


/*
 * Definición de la sección del puerto a utilizar.
 *
 * STEPPER_LOW_NIBBLE:
 * Utiliza los bits menos significativos del puerto:
 * bit 0, bit 1, bit 2 y bit 3.
 *
 * Ejemplo usando el puerto D:
 * RD0, RD1, RD2 y RD3.
 *
 * STEPPER_HIGH_NIBBLE:
 * Utiliza los bits más significativos del puerto:
 * bit 4, bit 5, bit 6 y bit 7.
 *
 * Ejemplo usando el puerto D:
 * RD4, RD5, RD6 y RD7.
 */
#define STEPPER_LOW_NIBBLE   0
#define STEPPER_HIGH_NIBBLE  1


/*
 * Cantidad aproximada de pasos necesarios para completar una vuelta
 * del eje externo del motor paso a paso 28BYJ-48 de 5 V.
 *
 * Aunque el motor interno tiene menos pasos por vuelta, el 28BYJ-48 incluye
 * una caja reductora interna. Debido a esta reducción mecánica, el eje externo
 * requiere aproximadamente 2048 pasos para completar una vuelta completa
 * cuando se trabaja con una secuencia de paso completo.
 *
 * Este valor se utiliza como referencia práctica para aplicaciones generales.
 */
#define STEPPER_STEPS_PER_REV_28BYJ48  2048


/*
 * Retardo por defecto entre pasos del motor, expresado en milisegundos.
 *
 * Este valor determina la velocidad de giro del motor. Un valor menor produce
 * mayor velocidad, pero si el retardo es demasiado bajo, el motor puede vibrar,
 * perder pasos o no girar correctamente.
 *
 * Para el motor 28BYJ-48 de 5 V con driver ULN2003, un valor inicial recomendado
 * es 5 ms.
 */
#define STEPPER_DEFAULT_DELAY_MS       5


/*
 * Estructura de configuración para un motor paso a paso.
 *
 * Cada motor que se desee controlar debe tener una variable propia de tipo
 * Stepper. Esta estructura almacena los registros del puerto, la sección del
 * puerto utilizada y la posición actual dentro de la secuencia de pasos.
 *
 * Campos:
 *
 * lat:
 * Puntero al registro LAT del puerto utilizado. Este registro se usa para
 * escribir los estados lógicos de salida.
 *
 * tris:
 * Puntero al registro TRIS del puerto utilizado. Este registro se usa para
 * configurar los pines como entradas o salidas.
 *
 * ansel:
 * Puntero al registro ANSEL del puerto utilizado. Este registro se usa para
 * configurar los pines como analógicos o digitales.
 *
 * nibble:
 * Define si el motor utiliza los bits 0 a 3 o los bits 4 a 7 del puerto.
 *
 * index:
 * Guarda la posición actual de la secuencia de paso completo.
 */
typedef struct
{
    volatile uint8_t *lat;
    volatile uint8_t *tris;
    volatile uint8_t *ansel;
    uint8_t nibble;
    uint8_t index;
} Stepper;


/*
 * Inicializa un motor paso a paso.
 *
 * Esta función configura los cuatro bits seleccionados como salidas digitales
 * y apaga inicialmente las salidas asignadas al motor.
 *
 * Parámetros:
 *
 * motor:
 * Dirección de la estructura Stepper asociada al motor.
 *
 * lat:
 * Dirección del registro LAT del puerto a utilizar.
 * Ejemplo: &LATD
 *
 * tris:
 * Dirección del registro TRIS del puerto a utilizar.
 * Ejemplo: &TRISD
 *
 * ansel:
 * Dirección del registro ANSEL del puerto a utilizar.
 * Ejemplo: &ANSELD
 *
 * nibble:
 * Define si se usará la parte baja o alta del puerto.
 * Puede tomar los valores:
 *
 * STEPPER_LOW_NIBBLE:
 * Usa los bits 0, 1, 2 y 3.
 *
 * STEPPER_HIGH_NIBBLE:
 * Usa los bits 4, 5, 6 y 7.
 *
 * Ejemplo:
 *
 * Stepper_Init(&motor1, &LATD, &TRISD, &ANSELD, STEPPER_LOW_NIBBLE);
 */
void Stepper_Init(Stepper *motor,
                  volatile uint8_t *lat,
                  volatile uint8_t *tris,
                  volatile uint8_t *ansel,
                  uint8_t nibble);


/*
 * Ejecuta un paso en sentido horario.
 *
 * Esta función aplica el siguiente estado de la secuencia de paso completo
 * al motor indicado.
 *
 * Parámetros:
 *
 * motor:
 * Dirección de la estructura Stepper asociada al motor.
 */
void Stepper_Step_CW(Stepper *motor);


/*
 * Ejecuta un paso en sentido antihorario.
 *
 * Esta función aplica el estado anterior de la secuencia de paso completo
 * al motor indicado.
 *
 * Parámetros:
 *
 * motor:
 * Dirección de la estructura Stepper asociada al motor.
 */
void Stepper_Step_CCW(Stepper *motor);


/*
 * Ejecuta una cantidad determinada de pasos en sentido horario.
 *
 * Esta función realiza varios pasos consecutivos en sentido horario,
 * aplicando un retardo entre cada paso.
 *
 * Parámetros:
 *
 * motor:
 * Dirección de la estructura Stepper asociada al motor.
 *
 * steps:
 * Número de pasos a ejecutar.
 *
 * delay_ms:
 * Tiempo de espera entre cada paso, expresado en milisegundos.
 */
void Stepper_Move_CW(Stepper *motor, uint16_t steps, uint16_t delay_ms);


/*
 * Ejecuta una cantidad determinada de pasos en sentido antihorario.
 *
 * Esta función realiza varios pasos consecutivos en sentido antihorario,
 * aplicando un retardo entre cada paso.
 *
 * Parámetros:
 *
 * motor:
 * Dirección de la estructura Stepper asociada al motor.
 *
 * steps:
 * Número de pasos a ejecutar.
 *
 * delay_ms:
 * Tiempo de espera entre cada paso, expresado en milisegundos.
 */
void Stepper_Move_CCW(Stepper *motor, uint16_t steps, uint16_t delay_ms);


/*
 * Ejecuta una vuelta completa del eje externo del motor 28BYJ-48
 * en sentido horario.
 *
 * Esta función utiliza los valores definidos por defecto:
 *
 * STEPPER_STEPS_PER_REV_28BYJ48
 * STEPPER_DEFAULT_DELAY_MS
 *
 * Equivale aproximadamente a ejecutar:
 *
 * Stepper_Move_CW(motor, 2048, 5);
 *
 * Parámetros:
 *
 * motor:
 * Dirección de la estructura Stepper asociada al motor.
 */
void Stepper_fullTurn_CW(Stepper *motor);


/*
 * Ejecuta una vuelta completa del eje externo del motor 28BYJ-48
 * en sentido antihorario.
 *
 * Esta función utiliza los valores definidos por defecto:
 *
 * STEPPER_STEPS_PER_REV_28BYJ48
 * STEPPER_DEFAULT_DELAY_MS
 *
 * Equivale aproximadamente a ejecutar:
 *
 * Stepper_Move_CCW(motor, 2048, 5);
 *
 * Parámetros:
 *
 * motor:
 * Dirección de la estructura Stepper asociada al motor.
 */
void Stepper_fullTurn_CCW(Stepper *motor);


/*
 * Desactiva las cuatro salidas asignadas al motor.
 *
 * Esta función coloca en estado bajo las cuatro señales utilizadas para
 * controlar el motor paso a paso.
 *
 * Parámetros:
 *
 * motor:
 * Dirección de la estructura Stepper asociada al motor.
 *
 * Consideración:
 * Al desactivar las salidas, el motor puede perder torque de retención.
 */
void Stepper_Off(Stepper *motor);

#endif