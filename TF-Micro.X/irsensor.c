#include "irsensor.h"

/*
 * Inicializa un sensor infrarrojo digital.
 *
 * La función recibe los registros del puerto donde está conectado el sensor
 * y configura el pin seleccionado como entrada digital.
 */
void IRSensor_Init(IRsensor *sensor,
                   volatile uint8_t *port,
                   volatile uint8_t *tris,
                   volatile uint8_t *ansel,
                   uint8_t pin_mask)
{
    /*
     * Guarda la dirección del registro PORT.
     *
     * PORT se utiliza para leer el estado lógico real del pin.
     */
    sensor->port = port;

    /*
     * Guarda la dirección del registro TRIS.
     *
     * TRIS se utiliza para configurar el pin como entrada o salida.
     */
    sensor->tris = tris;

    /*
     * Guarda la dirección del registro ANSEL.
     *
     * ANSEL se utiliza para configurar el pin como analógico o digital.
     */
    sensor->ansel = ansel;

    /*
     * Guarda la máscara del pin utilizado.
     */
    sensor->pin_mask = pin_mask;

    /*
     * Configura el pin seleccionado como digital.
     *
     * En ANSEL:
     * 0 = digital.
     * 1 = analógico.
     *
     * Se limpia únicamente el bit indicado por pin_mask.
     */
    *(sensor->ansel) = (uint8_t)(*(sensor->ansel) & (uint8_t)(~sensor->pin_mask));

    /*
     * Configura el pin seleccionado como entrada.
     *
     * En TRIS:
     * 0 = salida.
     * 1 = entrada.
     *
     * Se coloca en 1 únicamente el bit indicado por pin_mask.
     */
    *(sensor->tris) = (uint8_t)(*(sensor->tris) | sensor->pin_mask);
}


/*
 * Lee el sensor como activo en bajo.
 *
 * Esta función debe usarse cuando el módulo entrega nivel bajo al detectar.
 *
 * Ejemplo de comportamiento:
 *
 * Pin = 0 -> detecta.
 * Pin = 1 -> no detecta.
 */
uint8_t IRSensor_ReadActiveLow(IRsensor *sensor)
{
    /*
     * Si PORT & pin_mask es igual a 0, significa que el pin está en bajo.
     * Como el sensor es activo en bajo, se retorna 1 para indicar detección.
     */
    if((*(sensor->port) & sensor->pin_mask) == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*
 * Lee el sensor como activo en alto.
 *
 * Esta función debe usarse cuando el módulo entrega nivel alto al detectar.
 *
 * Ejemplo de comportamiento:
 *
 * Pin = 1 -> detecta.
 * Pin = 0 -> no detecta.
 */
uint8_t IRSensor_ReadActiveHigh(IRsensor *sensor)
{
    /*
     * Si PORT & pin_mask es diferente de 0, significa que el pin está en alto.
     * Como el sensor es activo en alto, se retorna 1 para indicar detección.
     */
    if((*(sensor->port) & sensor->pin_mask) != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}