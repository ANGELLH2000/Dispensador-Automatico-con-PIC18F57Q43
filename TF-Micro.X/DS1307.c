/**
 * @file DS1307.c
 * @brief Implementación de la librería de solo lectura DS1307.
 *
 * Microcontrolador objetivo: PIC18F57Q43.
 * Compilador: MPLAB XC8.
 */

#include <stddef.h>
#include <stdint.h>

#include "DS1307.h"


//==========================================================
// Prototipos privados
//==========================================================

static uint8_t DS1307_BCDToDecimal(uint8_t value_bcd);
static uint8_t DS1307_IsValidBCD(uint8_t value_bcd);
static uint8_t DS1307_ValidateRawData(const uint8_t *raw_data);


//==========================================================
// Conversión BCD
//==========================================================

/**
 * @brief Convierte un byte BCD en decimal.
 *
 * Ejemplo:
 *
 * 0x34 -> 34.
 */
static uint8_t DS1307_BCDToDecimal(uint8_t value_bcd)
{
    return (uint8_t)(
        (((value_bcd >> 4) & 0x0Fu) * 10u) +
        (value_bcd & 0x0Fu)
    );
}


/**
 * @brief Comprueba que ambos nibbles representen dígitos BCD.
 */
static uint8_t DS1307_IsValidBCD(uint8_t value_bcd)
{
    uint8_t tens;
    uint8_t units;

    tens = (uint8_t)(
        (value_bcd >> 4) & 0x0Fu
    );

    units = (uint8_t)(
        value_bcd & 0x0Fu
    );

    if ((tens > 9u) || (units > 9u))
    {
        return 0u;
    }

    return 1u;
}


//==========================================================
// Validación de registros
//==========================================================

static uint8_t DS1307_ValidateRawData(
    const uint8_t *raw_data
)
{
    uint8_t seconds_bcd;
    uint8_t minutes_bcd;
    uint8_t hours_bcd;
    uint8_t date_bcd;
    uint8_t month_bcd;
    uint8_t year_bcd;

    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t date;
    uint8_t month;
    uint8_t weekday;


    if (raw_data == NULL)
    {
        return 0u;
    }


    /*
     * Elimina los bits de control antes de evaluar el BCD.
     */
    seconds_bcd = (uint8_t)(
        raw_data[0] & 0x7Fu
    );

    minutes_bcd = (uint8_t)(
        raw_data[1] & 0x7Fu
    );

    date_bcd = (uint8_t)(
        raw_data[4] & 0x3Fu
    );

    month_bcd = (uint8_t)(
        raw_data[5] & 0x1Fu
    );

    year_bcd = raw_data[6];

    weekday = (uint8_t)(
        raw_data[3] & 0x07u
    );


    /*
     * Selecciona los bits correspondientes a las horas.
     */
    if ((raw_data[2] & 0x40u) != 0u)
    {
        /*
         * Formato de doce horas.
         */
        hours_bcd = (uint8_t)(
            raw_data[2] & 0x1Fu
        );
    }
    else
    {
        /*
         * Formato de veinticuatro horas.
         */
        hours_bcd = (uint8_t)(
            raw_data[2] & 0x3Fu
        );
    }


    /*
     * Comprueba que cada nibble sea un dígito BCD.
     */
    if ((DS1307_IsValidBCD(seconds_bcd) == 0u) ||
        (DS1307_IsValidBCD(minutes_bcd) == 0u) ||
        (DS1307_IsValidBCD(hours_bcd) == 0u)   ||
        (DS1307_IsValidBCD(date_bcd) == 0u)    ||
        (DS1307_IsValidBCD(month_bcd) == 0u)   ||
        (DS1307_IsValidBCD(year_bcd) == 0u))
    {
        return 0u;
    }


    seconds = DS1307_BCDToDecimal(seconds_bcd);
    minutes = DS1307_BCDToDecimal(minutes_bcd);
    hours   = DS1307_BCDToDecimal(hours_bcd);
    date    = DS1307_BCDToDecimal(date_bcd);
    month   = DS1307_BCDToDecimal(month_bcd);


    if (seconds > 59u)
    {
        return 0u;
    }

    if (minutes > 59u)
    {
        return 0u;
    }


    /*
     * Validación según el modo de hora.
     */
    if ((raw_data[2] & 0x40u) != 0u)
    {
        if ((hours == 0u) || (hours > 12u))
        {
            return 0u;
        }
    }
    else
    {
        if (hours > 23u)
        {
            return 0u;
        }
    }


    if ((weekday == 0u) || (weekday > 7u))
    {
        return 0u;
    }

    if ((date == 0u) || (date > 31u))
    {
        return 0u;
    }

    if ((month == 0u) || (month > 12u))
    {
        return 0u;
    }


    return 1u;
}


//==========================================================
// Lectura de registros
//==========================================================

I2C_Status DS1307_ReadRegisters(
    uint8_t start_register,
    uint8_t *data,
    uint8_t length
)
{
    I2C_Status status;


    if ((start_register > 0x3Fu) ||
        (data == NULL) ||
        (length == 0u))
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    /*
     * Esta operación no modifica los datos del RTC.
     *
     * Solamente coloca el puntero interno en el registro
     * desde donde comenzará la lectura.
     */
    status = I2C1_WriteSingleByte(
        DS1307_I2C_ADDRESS,
        start_register
    );


    if (status != I2C_OK)
    {
        return status;
    }


    /*
     * Lee los registros consecutivos.
     */
    return I2C1_Read(
        DS1307_I2C_ADDRESS,
        data,
        length
    );
}


//==========================================================
// Lectura cruda
//==========================================================

I2C_Status DS1307_ReadRaw(
    uint8_t raw_data[DS1307_TIME_REGISTER_COUNT]
)
{
    if (raw_data == NULL)
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    return DS1307_ReadRegisters(
        DS1307_REG_SECONDS,
        raw_data,
        DS1307_TIME_REGISTER_COUNT
    );
}


//==========================================================
// Lectura y conversión de fecha y hora
//==========================================================

I2C_Status DS1307_ReadDateTime(
    DS1307_DateTime *date_time
)
{
    uint8_t raw_data[DS1307_TIME_REGISTER_COUNT];
    I2C_Status status;


    if (date_time == NULL)
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    /*
     * Inicializa los indicadores antes de leer.
     */
    date_time->clock_running = 0u;
    date_time->mode_12h = 0u;
    date_time->is_pm = 0u;
    date_time->data_valid = 0u;


    /*
     * Lee los siete registros de tiempo.
     */
    status = DS1307_ReadRaw(raw_data);


    if (status != I2C_OK)
    {
        return status;
    }


    /*
     * Bit CH:
     *
     * CH = 0 -> oscilador funcionando.
     * CH = 1 -> oscilador detenido.
     */
    date_time->clock_running =
        ((raw_data[0] & 0x80u) == 0u) ? 1u : 0u;


    /*
     * Bit 6 del registro de horas:
     *
     * 0 -> formato de veinticuatro horas.
     * 1 -> formato de doce horas.
     */
    date_time->mode_12h =
        ((raw_data[2] & 0x40u) != 0u) ? 1u : 0u;


    if (date_time->mode_12h != 0u)
    {
        /*
         * Bit 5:
         *
         * 0 -> AM.
         * 1 -> PM.
         */
        date_time->is_pm =
            ((raw_data[2] & 0x20u) != 0u) ? 1u : 0u;


        date_time->hours = DS1307_BCDToDecimal(
            (uint8_t)(raw_data[2] & 0x1Fu)
        );
    }
    else
    {
        date_time->is_pm = 0u;


        date_time->hours = DS1307_BCDToDecimal(
            (uint8_t)(raw_data[2] & 0x3Fu)
        );
    }


    date_time->seconds = DS1307_BCDToDecimal(
        (uint8_t)(raw_data[0] & 0x7Fu)
    );


    date_time->minutes = DS1307_BCDToDecimal(
        (uint8_t)(raw_data[1] & 0x7Fu)
    );


    date_time->weekday = (uint8_t)(
        raw_data[3] & 0x07u
    );


    date_time->date = DS1307_BCDToDecimal(
        (uint8_t)(raw_data[4] & 0x3Fu)
    );


    date_time->month = DS1307_BCDToDecimal(
        (uint8_t)(raw_data[5] & 0x1Fu)
    );


    date_time->year = (uint16_t)(
        2000u +
        DS1307_BCDToDecimal(raw_data[6])
    );


    /*
     * Comprueba que la información recibida tenga
     * valores BCD y rangos correctos.
     */
    date_time->data_valid =
        DS1307_ValidateRawData(raw_data);


    return I2C_OK;
}