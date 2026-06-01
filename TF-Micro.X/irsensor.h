#ifndef IRSENSOR_H
#define IRSENSOR_H

#include "cabecera.h"

/*
 * Librería para lectura de sensor infrarrojo digital.
 *
 * Esta librería permite leer sensores infrarrojos digitales con salida lógica,
 * como módulos de obstáculo IR basados en comparador LM393.
 *
 * Algunos sensores entregan nivel bajo cuando detectan.
 * Otros sensores entregan nivel alto cuando detectan.
 *
 * Por ello, la librería proporciona dos funciones de lectura:
 *
 * IRSensor_ReadActiveLow():
 * Retorna 1 cuando el sensor detecta en nivel bajo.
 *
 * IRSensor_ReadActiveHigh():
 * Retorna 1 cuando el sensor detecta en nivel alto.
 */


/*
 * Estructura de configuración de un sensor infrarrojo digital.
 *
 * Cada sensor que se desee utilizar debe tener una variable propia
 * de tipo IRsensor.
 *
 * Campos:
 *
 * port:
 * Puntero al registro PORT del puerto utilizado.
 * Se usa para leer el estado lógico real del pin.
 *
 * tris:
 * Puntero al registro TRIS del puerto utilizado.
 * Se usa para configurar el pin como entrada.
 *
 * ansel:
 * Puntero al registro ANSEL del puerto utilizado.
 * Se usa para configurar el pin como digital.
 *
 * pin_mask:
 * Máscara del pin donde está conectada la salida digital del sensor.
 */
typedef struct
{
    volatile uint8_t *port;
    volatile uint8_t *tris;
    volatile uint8_t *ansel;

    uint8_t pin_mask;

} IRsensor;


/*
 * Inicializa un sensor infrarrojo digital.
 *
 * Esta función configura el pin seleccionado como entrada digital y guarda
 * la configuración dentro de la estructura asociada al sensor.
 *
 * Parámetros:
 *
 * sensor:
 * Dirección de la estructura IRsensor asociada al sensor.
 *
 * port:
 * Dirección del registro PORT del puerto utilizado.
 * Ejemplo: &PORTD
 *
 * tris:
 * Dirección del registro TRIS del puerto utilizado.
 * Ejemplo: &TRISD
 *
 * ansel:
 * Dirección del registro ANSEL del puerto utilizado.
 * Ejemplo: &ANSELD
 *
 * pin_mask:
 * Máscara del pin donde está conectada la salida digital del sensor.
 *
 * Ejemplos para el puerto D:
 *
 * RD0 -> 0x01
 * RD1 -> 0x02
 * RD2 -> 0x04
 * RD3 -> 0x08
 * RD4 -> 0x10
 * RD5 -> 0x20
 * RD6 -> 0x40
 * RD7 -> 0x80
 *
 * Ejemplo:
 *
 * IRsensor sensor1;
 * IRSensor_Init(&sensor1, &PORTD, &TRISD, &ANSELD, 0x01);
 */
void IRSensor_Init(IRsensor *sensor,
                   volatile uint8_t *port,
                   volatile uint8_t *tris,
                   volatile uint8_t *ansel,
                   uint8_t pin_mask);


/*
 * Lee el sensor como activo en bajo.
 *
 * Esta función se usa cuando el sensor entrega 0 al detectar.
 *
 * Retorna:
 *
 * 1:
 * El sensor detecta.
 *
 * 0:
 * El sensor no detecta.
 */
uint8_t IRSensor_ReadActiveLow(IRsensor *sensor);


/*
 * Lee el sensor como activo en alto.
 *
 * Esta función se usa cuando el sensor entrega 1 al detectar.
 *
 * Retorna:
 *
 * 1:
 * El sensor detecta.
 *
 * 0:
 * El sensor no detecta.
 */
uint8_t IRSensor_ReadActiveHigh(IRsensor *sensor);

#endif