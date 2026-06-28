#ifndef DS1307_H
#define DS1307_H

/**
 * @file DS1307.h
 * @brief Librería de solo lectura para el RTC DS1307.
 *
 * Esta librería utiliza la capa genérica definida en I2C.h.
 * No configura ni modifica la fecha y hora almacenadas.
 */

#include <stdint.h>

#include "I2C.h"


//==========================================================
// Constantes del dispositivo
//==========================================================

/**
 * Dirección I2C de siete bits del DS1307.
 *
 * La librería I2C agrega automáticamente el bit R/W.
 */
#define DS1307_I2C_ADDRESS          0x68


/**
 * Direcciones de los registros.
 */
#define DS1307_REG_SECONDS          0x00u
#define DS1307_REG_MINUTES          0x01u
#define DS1307_REG_HOURS            0x02u
#define DS1307_REG_WEEKDAY          0x03u
#define DS1307_REG_DATE             0x04u
#define DS1307_REG_MONTH            0x05u
#define DS1307_REG_YEAR             0x06u
#define DS1307_REG_CONTROL          0x07u

#define DS1307_TIME_REGISTER_COUNT  7u


//==========================================================
// Estructura de fecha y hora
//==========================================================

/**
 * @brief Fecha y hora decodificadas del DS1307.
 *
 * Todos los campos numéricos se entregan en decimal.
 */
typedef struct
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;

    uint8_t weekday;
    uint8_t date;
    uint8_t month;
    uint16_t year;

    /**
     * 1: el bit CH está en cero y el reloj funciona.
     * 0: el bit CH está en uno y el reloj está detenido.
     */
    uint8_t clock_running;

    /**
     * 1: formato de doce horas.
     * 0: formato de veinticuatro horas.
     */
    uint8_t mode_12h;

    /**
     * Solo se utiliza cuando mode_12h = 1.
     *
     * 1: PM.
     * 0: AM.
     */
    uint8_t is_pm;

    /**
     * 1: los registros contienen datos válidos.
     * 0: los registros contienen valores inválidos.
     */
    uint8_t data_valid;

} DS1307_DateTime;


//==========================================================
// Funciones públicas
//==========================================================

/**
 * @brief Lee uno o varios registros consecutivos.
 *
 * @param start_register Primer registro que será leído.
 * @param data Arreglo donde se guardarán los datos.
 * @param length Cantidad de registros que deben leerse.
 *
 * @return Estado final de la comunicación I2C.
 *
 * Esta función no modifica la fecha ni la hora. El envío de
 * start_register solamente coloca el puntero interno del
 * DS1307 en la posición desde donde comenzará la lectura.
 */
I2C_Status DS1307_ReadRegisters(
    uint8_t start_register,
    uint8_t *data,
    uint8_t length
);


/**
 * @brief Lee los siete registros de fecha y hora sin convertir.
 *
 * Orden:
 *
 * [0] segundos
 * [1] minutos
 * [2] horas
 * [3] día de la semana
 * [4] día del mes
 * [5] mes
 * [6] año
 */
I2C_Status DS1307_ReadRaw(
    uint8_t raw_data[DS1307_TIME_REGISTER_COUNT]
);


/**
 * @brief Lee y convierte la fecha y hora a decimal.
 *
 * @param date_time Estructura donde se guardarán los datos.
 *
 * @return Estado final de la comunicación I2C.
 *
 * Si la comunicación termina correctamente, pero los registros
 * contienen información inválida, devuelve I2C_OK y establece:
 *
 * date_time->data_valid = 0.
 */
I2C_Status DS1307_ReadDateTime(
    DS1307_DateTime *date_time
);


#endif /* DS1307_H */