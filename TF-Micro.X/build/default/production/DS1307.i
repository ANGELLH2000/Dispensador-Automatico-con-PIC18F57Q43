# 1 "DS1307.c"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 295 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include/language_support.h" 1 3
# 2 "<built-in>" 2
# 1 "DS1307.c" 2








# 1 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/stddef.h" 1 3



# 1 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/musl_xc8.h" 1 3
# 5 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/stddef.h" 2 3
# 19 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/stddef.h" 3
# 1 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 1 3
# 24 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 3
typedef long int wchar_t;
# 128 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 3
typedef unsigned size_t;
# 138 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 3
typedef int ptrdiff_t;
# 174 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 3
typedef __int24 int24_t;
# 210 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 3
typedef __uint24 uint24_t;
# 20 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/stddef.h" 2 3
# 10 "DS1307.c" 2
# 1 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/stdint.h" 1 3
# 26 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/stdint.h" 3
# 1 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 1 3
# 133 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 3
typedef unsigned __int24 uintptr_t;
# 148 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 3
typedef __int24 intptr_t;
# 164 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 3
typedef signed char int8_t;




typedef short int16_t;
# 179 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 3
typedef long int32_t;





typedef long long int64_t;
# 194 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 3
typedef long long intmax_t;





typedef unsigned char uint8_t;




typedef unsigned short uint16_t;
# 215 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 3
typedef unsigned long uint32_t;





typedef unsigned long long uint64_t;
# 235 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/alltypes.h" 3
typedef unsigned long long uintmax_t;
# 27 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/stdint.h" 2 3

typedef int8_t int_fast8_t;

typedef int64_t int_fast64_t;


typedef int8_t int_least8_t;
typedef int16_t int_least16_t;

typedef int24_t int_least24_t;
typedef int24_t int_fast24_t;

typedef int32_t int_least32_t;

typedef int64_t int_least64_t;


typedef uint8_t uint_fast8_t;

typedef uint64_t uint_fast64_t;


typedef uint8_t uint_least8_t;
typedef uint16_t uint_least16_t;

typedef uint24_t uint_least24_t;
typedef uint24_t uint_fast24_t;

typedef uint32_t uint_least32_t;

typedef uint64_t uint_least64_t;
# 148 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/stdint.h" 3
# 1 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/bits/stdint.h" 1 3
typedef int16_t int_fast16_t;
typedef int32_t int_fast32_t;
typedef uint16_t uint_fast16_t;
typedef uint32_t uint_fast32_t;
# 149 "C:\\Program Files\\Microchip\\xc8\\v3.10\\pic\\include\\c99/stdint.h" 2 3
# 11 "DS1307.c" 2

# 1 "./DS1307.h" 1
# 14 "./DS1307.h"
# 1 "./I2C.h" 1
# 29 "./I2C.h"
typedef enum
{
    I2C_OK = 0,
# 42 "./I2C.h"
    I2C_ERROR_NACK,




    I2C_ERROR_COLLISION,




    I2C_ERROR_BUS_TIMEOUT,




    I2C_ERROR_SOFTWARE_TIMEOUT,




    I2C_ERROR_INVALID_PARAMETER

} I2C_Status;
# 74 "./I2C.h"
extern volatile I2C_Status I2C1_LastStatus;
# 93 "./I2C.h"
void I2C1_Init(void);
# 114 "./I2C.h"
I2C_Status I2C1_Write(
    uint8_t address7,
    const uint8_t *data,
    uint8_t length
);
# 129 "./I2C.h"
I2C_Status I2C1_WriteSingleByte(
    uint8_t address7,
    uint8_t data
);
# 151 "./I2C.h"
I2C_Status I2C1_Read(
    uint8_t address7,
    uint8_t *data,
    uint8_t length
);
# 15 "./DS1307.h" 2
# 53 "./DS1307.h"
typedef struct
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;

    uint8_t weekday;
    uint8_t date;
    uint8_t month;
    uint16_t year;





    uint8_t clock_running;





    uint8_t mode_12h;







    uint8_t is_pm;





    uint8_t data_valid;

} DS1307_DateTime;
# 110 "./DS1307.h"
I2C_Status DS1307_ReadRegisters(
    uint8_t start_register,
    uint8_t *data,
    uint8_t length
);
# 130 "./DS1307.h"
I2C_Status DS1307_ReadRaw(
    uint8_t raw_data[7u]
);
# 147 "./DS1307.h"
I2C_Status DS1307_ReadDateTime(
    DS1307_DateTime *date_time
);
# 13 "DS1307.c" 2






static uint8_t DS1307_BCDToDecimal(uint8_t value_bcd);
static uint8_t DS1307_IsValidBCD(uint8_t value_bcd);
static uint8_t DS1307_ValidateRawData(const uint8_t *raw_data);
# 35 "DS1307.c"
static uint8_t DS1307_BCDToDecimal(uint8_t value_bcd)
{
    return (uint8_t)(
        (((value_bcd >> 4) & 0x0Fu) * 10u) +
        (value_bcd & 0x0Fu)
    );
}





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


    if (raw_data == ((void*)0))
    {
        return 0u;
    }





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





    if ((raw_data[2] & 0x40u) != 0u)
    {



        hours_bcd = (uint8_t)(
            raw_data[2] & 0x1Fu
        );
    }
    else
    {



        hours_bcd = (uint8_t)(
            raw_data[2] & 0x3Fu
        );
    }





    if ((DS1307_IsValidBCD(seconds_bcd) == 0u) ||
        (DS1307_IsValidBCD(minutes_bcd) == 0u) ||
        (DS1307_IsValidBCD(hours_bcd) == 0u) ||
        (DS1307_IsValidBCD(date_bcd) == 0u) ||
        (DS1307_IsValidBCD(month_bcd) == 0u) ||
        (DS1307_IsValidBCD(year_bcd) == 0u))
    {
        return 0u;
    }


    seconds = DS1307_BCDToDecimal(seconds_bcd);
    minutes = DS1307_BCDToDecimal(minutes_bcd);
    hours = DS1307_BCDToDecimal(hours_bcd);
    date = DS1307_BCDToDecimal(date_bcd);
    month = DS1307_BCDToDecimal(month_bcd);


    if (seconds > 59u)
    {
        return 0u;
    }

    if (minutes > 59u)
    {
        return 0u;
    }





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






I2C_Status DS1307_ReadRegisters(
    uint8_t start_register,
    uint8_t *data,
    uint8_t length
)
{
    I2C_Status status;


    if ((start_register > 0x3Fu) ||
        (data == ((void*)0)) ||
        (length == 0u))
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }
# 245 "DS1307.c"
    status = I2C1_WriteSingleByte(
        0x68,
        start_register
    );


    if (status != I2C_OK)
    {
        return status;
    }





    return I2C1_Read(
        0x68,
        data,
        length
    );
}






I2C_Status DS1307_ReadRaw(
    uint8_t raw_data[7u]
)
{
    if (raw_data == ((void*)0))
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }


    return DS1307_ReadRegisters(
        0x00u,
        raw_data,
        7u
    );
}






I2C_Status DS1307_ReadDateTime(
    DS1307_DateTime *date_time
)
{
    uint8_t raw_data[7u];
    I2C_Status status;


    if (date_time == ((void*)0))
    {
        return I2C_ERROR_INVALID_PARAMETER;
    }





    date_time->clock_running = 0u;
    date_time->mode_12h = 0u;
    date_time->is_pm = 0u;
    date_time->data_valid = 0u;





    status = DS1307_ReadRaw(raw_data);


    if (status != I2C_OK)
    {
        return status;
    }
# 335 "DS1307.c"
    date_time->clock_running =
        ((raw_data[0] & 0x80u) == 0u) ? 1u : 0u;
# 345 "DS1307.c"
    date_time->mode_12h =
        ((raw_data[2] & 0x40u) != 0u) ? 1u : 0u;


    if (date_time->mode_12h != 0u)
    {






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






    date_time->data_valid =
        DS1307_ValidateRawData(raw_data);


    return I2C_OK;
}
