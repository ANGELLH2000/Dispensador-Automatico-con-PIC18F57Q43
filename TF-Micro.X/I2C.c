/**
 * @file I2C.c
 * @brief Implementación del controlador genérico I2C1.
 *
 * Microcontrolador objetivo: PIC18F57Q43.
 * Compilador: MPLAB XC8.
 *
 * La librería permite:
 *
 * - Inicializar I2C1 como maestro.
 * - Escribir uno o varios bytes.
 * - Leer uno o varios bytes.
 * - Detectar ACK y NACK.
 * - Detectar colisiones y timeout.
 * - Recuperar el módulo cuando un dispositivo no responde.
 */

#include <xc.h>
#include <stddef.h>
#include <stdint.h>

#include "I2C.h"


//==========================================================
// Constantes internas
//==========================================================

/**
 * Número máximo de iteraciones utilizado en las esperas
 * mediante polling.
 *
 * Este valor no representa microsegundos. Es solamente una
 * protección para evitar que el programa permanezca bloqueado
 * indefinidamente.
 */
#define I2C_SOFTWARE_TIMEOUT  60000u


//==========================================================
// Variable pública de estado
//==========================================================

/**
 * Último resultado producido por el controlador I2C1.
 */
volatile I2C_Status I2C1_LastStatus = I2C_OK;


//==========================================================
// Prototipos privados
//==========================================================

static void I2C1_ClearFlags(void);
static void I2C1_ResetModule(void);
static void I2C1_ConfigurePPS(void);

static I2C_Status I2C1_WaitBusFree(void);
static I2C_Status I2C1_WaitTxBufferEmpty(void);
static I2C_Status I2C1_WaitRxBufferFull(void);
static I2C_Status I2C1_WaitStop(void);
static I2C_Status I2C1_WaitStopRead(void);


//==========================================================
// Limpieza y recuperación del periférico
//==========================================================

/**
 * @brief Limpia las banderas de eventos y errores.
 */
static void I2C1_ClearFlags(void)
{
    /*
     * Limpia las banderas generales del módulo.
     */
    I2C1PIR = 0x00u;

    /*
     * Limpia:
     *
     * BTOIF  -> timeout del bus.
     * BCLIF  -> colisión.
     * NACKIF -> NACK recibido.
     */
    I2C1ERR = 0x00u;
}


/**
 * @brief Reinicia la máquina de estados de I2C1.
 *
 * Se utiliza después de cualquier error para permitir que
 * otros dispositivos del bus continúen funcionando.
 *
 * Esto es especialmente importante cuando se intenta acceder
 * a un dispositivo que todavía no está conectado.
 */
static void I2C1_ResetModule(void)
{
    /*
     * Deshabilita temporalmente el periférico.
     */
    I2C1CON0bits.EN = 0u;

    /*
     * Limpia todas las banderas.
     */
    I2C1_ClearFlags();

    /*
     * Vuelve a habilitar el periférico.
     *
     * Los registros de configuración conservan los valores
     * establecidos en I2C1_Init().
     */
    I2C1CON0bits.EN = 1u;
}


//==========================================================
// Configuración PPS
//==========================================================

/**
 * @brief Conecta el periférico I2C1 con RC3 y RC4.
 *
 * SCL utiliza RC3.
 * SDA utiliza RC4.
 *
 * Debido a que ambas señales son bidireccionales, deben
 * configurarse los PPS de entrada y de salida.
 */
static void I2C1_ConfigurePPS(void)
{
    uint8_t previousGie;

    /*
     * Guarda el estado actual de las interrupciones globales.
     */
    previousGie = INTCON0bits.GIE;

    /*
     * Deshabilita temporalmente las interrupciones.
     */
    INTCON0bits.GIE = 0u;


    //======================================================
    // Desbloqueo de PPS
    //======================================================

    PPSLOCK = 0x55u;
    PPSLOCK = 0xAAu;

    PPSLOCKbits.PPSLOCKED = 0u;


    //======================================================
    // PPS de salida
    //======================================================

    /*
     * RC3 recibe la salida SCL del periférico I2C1.
     */
    RC3PPS = 0x37u;

    /*
     * RC4 recibe la salida SDA del periférico I2C1.
     */
    RC4PPS = 0x38u;


    //======================================================
    // PPS de entrada
    //======================================================

    /*
     * La entrada SCL de I2C1 proviene de RC3.
     */
    I2C1SCLPPS = 0x13u;

    /*
     * La entrada SDA de I2C1 proviene de RC4.
     */
    I2C1SDAPPS = 0x14u;


    //======================================================
    // Bloqueo de PPS
    //======================================================

    PPSLOCK = 0x55u;
    PPSLOCK = 0xAAu;

    PPSLOCKbits.PPSLOCKED = 1u;


    /*
     * Restaura el estado anterior de las interrupciones.
     */
    INTCON0bits.GIE = previousGie;
}


//==========================================================
// Funciones privadas de espera
//==========================================================

/**
 * @brief Espera hasta que SDA y SCL indiquen bus libre.
 */
static I2C_Status I2C1_WaitBusFree(void)
{
    uint16_t timeout = I2C_SOFTWARE_TIMEOUT;

    /*
     * BFRE = 1 cuando el bus se encuentra libre.
     */
    while (I2C1STAT0bits.BFRE == 0u)
    {
        if (I2C1ERRbits.BTOIF != 0u)
        {
            return I2C_ERROR_BUS_TIMEOUT;
        }

        if (I2C1ERRbits.BCLIF != 0u)
        {
            return I2C_ERROR_COLLISION;
        }

        if (timeout == 0u)
        {
            return I2C_ERROR_SOFTWARE_TIMEOUT;
        }

        timeout--;
    }

    return I2C_OK;
}


/**
 * @brief Espera hasta que I2C1TXB pueda recibir otro byte.
 */
static I2C_Status I2C1_WaitTxBufferEmpty(void)
{
    uint16_t timeout = I2C_SOFTWARE_TIMEOUT;

    /*
     * TXBE = 1 cuando el búfer de transmisión está vacío.
     */
    while (I2C1STAT1bits.TXBE == 0u)
    {
        /*
         * El dispositivo no reconoció la dirección
         * o rechazó uno de los bytes transmitidos.
         */
        if (I2C1ERRbits.NACKIF != 0u)
        {
            return I2C_ERROR_NACK;
        }

        if (I2C1ERRbits.BTOIF != 0u)
        {
            return I2C_ERROR_BUS_TIMEOUT;
        }

        if (I2C1ERRbits.BCLIF != 0u)
        {
            return I2C_ERROR_COLLISION;
        }

        if (timeout == 0u)
        {
            return I2C_ERROR_SOFTWARE_TIMEOUT;
        }

        timeout--;
    }

    return I2C_OK;
}


/**
 * @brief Espera hasta que exista un byte disponible.
 */
static I2C_Status I2C1_WaitRxBufferFull(void)
{
    uint16_t timeout = I2C_SOFTWARE_TIMEOUT;

    /*
     * RXBF = 1 cuando existe un byte en I2C1RXB.
     */
    while (I2C1STAT1bits.RXBF == 0u)
    {
        /*
         * El dispositivo no reconoció la dirección de lectura.
         */
        if (I2C1ERRbits.NACKIF != 0u)
        {
            return I2C_ERROR_NACK;
        }

        if (I2C1ERRbits.BTOIF != 0u)
        {
            return I2C_ERROR_BUS_TIMEOUT;
        }

        if (I2C1ERRbits.BCLIF != 0u)
        {
            return I2C_ERROR_COLLISION;
        }

        if (timeout == 0u)
        {
            return I2C_ERROR_SOFTWARE_TIMEOUT;
        }

        timeout--;
    }

    return I2C_OK;
}


/**
 * @brief Espera la condición Stop de una escritura.
 *
 * Al finalizar también comprueba que el dispositivo haya
 * respondido con ACK.
 */
static I2C_Status I2C1_WaitStop(void)
{
    uint16_t timeout = I2C_SOFTWARE_TIMEOUT;

    /*
     * PCIF se activa cuando finaliza la condición Stop.
     */
    while (I2C1PIRbits.PCIF == 0u)
    {
        if (I2C1ERRbits.BTOIF != 0u)
        {
            return I2C_ERROR_BUS_TIMEOUT;
        }

        if (I2C1ERRbits.BCLIF != 0u)
        {
            return I2C_ERROR_COLLISION;
        }

        if (timeout == 0u)
        {
            return I2C_ERROR_SOFTWARE_TIMEOUT;
        }

        timeout--;
    }

    /*
     * ACKSTAT = 1 o NACKIF = 1 indican que el dispositivo
     * no reconoció la operación.
     */
    if ((I2C1CON1bits.ACKSTAT != 0u) ||
        (I2C1ERRbits.NACKIF != 0u))
    {
        return I2C_ERROR_NACK;
    }

    return I2C_OK;
}


/**
 * @brief Espera la condición Stop de una lectura.
 *
 * No se comprueba ACKSTAT porque, durante una lectura, el
 * maestro envía intencionalmente un NACK después del último
 * byte para indicar que no desea recibir más datos.
 */
static I2C_Status I2C1_WaitStopRead(void)
{
    uint16_t timeout = I2C_SOFTWARE_TIMEOUT;

    while (I2C1PIRbits.PCIF == 0u)
    {
        if (I2C1ERRbits.BTOIF != 0u)
        {
            return I2C_ERROR_BUS_TIMEOUT;
        }

        if (I2C1ERRbits.BCLIF != 0u)
        {
            return I2C_ERROR_COLLISION;
        }

        if (timeout == 0u)
        {
            return I2C_ERROR_SOFTWARE_TIMEOUT;
        }

        timeout--;
    }

    return I2C_OK;
}


//==========================================================
// Inicialización del periférico
//==========================================================

void I2C1_Init(void)
{
    /*
     * El módulo permanece deshabilitado mientras se configura.
     */
    I2C1CON0bits.EN = 0u;


    //======================================================
    // Configuración de RC3 y RC4
    //======================================================

    /*
     * Inicializa los latches de salida.
     *
     * RC3 -> SCL.
     * RC4 -> SDA.
     */
    LATCbits.LATC3 = 0u;
    LATCbits.LATC4 = 0u;


    /*
     * Deshabilita las funciones analógicas.
     */
    ANSELCbits.ANSELC3 = 0u;
    ANSELCbits.ANSELC4 = 0u;


    /*
     * Los pines se configuran como salidas.
     *
     * El periférico I2C se encarga de llevar las líneas a
     * nivel bajo o de liberarlas mediante open-drain.
     */
    TRISCbits.TRISC3 = 0u;
    TRISCbits.TRISC4 = 0u;


    /*
     * Habilita el funcionamiento open-drain.
     */
    ODCONCbits.ODCC3 = 1u;
    ODCONCbits.ODCC4 = 1u;


    /*
     * Configura las conexiones PPS.
     */
    I2C1_ConfigurePPS();


    //======================================================
    // Configuración eléctrica de los pads
    //======================================================

    /*
     * Deshabilita los pull-up internos.
     *
     * Se utilizarán las resistencias pull-up externas de
     * los módulos conectados al bus.
     */
    RC3I2Cbits.PU = 0u;
    RC4I2Cbits.PU = 0u;


    /*
     * Selecciona umbrales específicos para I2C.
     */
    RC3I2Cbits.TH = 1u;
    RC4I2Cbits.TH = 1u;


    //======================================================
    // Configuración del modo I2C
    //======================================================

    /*
     * I2C1CON0:
     *
     * EN   = 0.
     * RSEN = 0.
     * S    = 0.
     * CSTR = 0.
     * MDR  = 0.
     * MODE = 100.
     *
     * MODE = 100:
     * maestro con direccionamiento de 7 bits.
     */
    I2C1CON0 = 0x04u;


    /*
     * Configuración inicial normal.
     *
     * ACKCNT se modificará antes de las lecturas.
     */
    I2C1CON1 = 0x00u;


    /*
     * I2C1CON2:
     *
     * ACNT = 0:
     * I2C1CNT se carga manualmente.
     *
     * GCEN = 0:
     * General Call deshabilitado.
     *
     * FME = 0:
     * SCL utiliza cinco periodos de I2C1CLK.
     *
     * ABD = 0:
     * dirección y datos utilizan registros separados.
     *
     * SDAHT = 00.
     * BFRET = 00.
     */
    I2C1CON2 = 0x00u;


    //======================================================
    // Reloj de I2C1
    //======================================================

    /*
     * Fuente del módulo:
     *
     * MFINTOSC = 500 kHz.
     *
     * Como FME = 0:
     *
     * fSCL = 500 kHz / 5
     * fSCL = 100 kHz.
     */
    I2C1CLK = 0x03u;


    /*
     * Timeout de hardware deshabilitado.
     */
    I2C1BTO = 0x00u;


    /*
     * Interrupciones del módulo deshabilitadas.
     *
     * La librería trabaja mediante polling.
     */
    I2C1PIE = 0x00u;


    /*
     * Limpia todas las banderas antes de habilitar I2C1.
     */
    I2C1_ClearFlags();


    /*
     * Habilita el periférico.
     */
    I2C1CON0bits.EN = 1u;


    /*
     * Estado inicial correcto.
     */
    I2C1_LastStatus = I2C_OK;
}


//==========================================================
// Escritura de un bloque
//==========================================================

I2C_Status I2C1_Write(
    uint8_t address7,
    const uint8_t *data,
    uint8_t length
)
{
    uint8_t index;
    I2C_Status status;


    //======================================================
    // Validación de parámetros
    //======================================================

    if ((address7 > 0x7Fu) ||
        (data == NULL) ||
        (length == 0u))
    {
        I2C1_LastStatus = I2C_ERROR_INVALID_PARAMETER;

        return I2C1_LastStatus;
    }


    //======================================================
    // Espera del bus
    //======================================================

    status = I2C1_WaitBusFree();

    if (status != I2C_OK)
    {
        I2C1_LastStatus = status;

        I2C1_ResetModule();

        return status;
    }


    /*
     * Limpia las banderas de una operación anterior.
     */
    I2C1_ClearFlags();


    /*
     * La operación debe finalizar con una condición Stop.
     */
    I2C1CON0bits.RSEN = 0u;


    /*
     * Dirección de siete bits más R/W = 0.
     *
     * Ejemplo para el DS1307:
     *
     * 0x68 << 1 = 0xD0.
     */
    I2C1ADB1 = (uint8_t)(address7 << 1);


    /*
     * Número de bytes que serán transmitidos.
     *
     * La dirección del esclavo no se incluye.
     */
    I2C1CNT = length;


    /*
     * El primer byte debe cargarse antes de solicitar Start.
     */
    I2C1TXB = data[0];


    /*
     * Inicia la transacción.
     */
    I2C1CON0bits.S = 1u;


    //======================================================
    // Envío de los bytes restantes
    //======================================================

    for (index = 1u; index < length; index++)
    {
        /*
         * Espera hasta que TXB pueda recibir otro byte.
         */
        status = I2C1_WaitTxBufferEmpty();

        if (status != I2C_OK)
        {
            /*
             * Recupera el periférico ante cualquier error,
             * incluido NACK.
             */
            I2C1_LastStatus = status;

            I2C1_ResetModule();

            return status;
        }

        /*
         * Carga el siguiente byte.
         */
        I2C1TXB = data[index];
    }


    //======================================================
    // Finalización
    //======================================================

    /*
     * Cuando I2C1CNT llega a cero, el hardware genera Stop.
     */
    status = I2C1_WaitStop();


    /*
     * Si se produjo cualquier error, incluido NACK,
     * se reinicia el periférico.
     */
    if (status != I2C_OK)
    {
        I2C1_ResetModule();
    }
    else
    {
        I2C1_ClearFlags();
    }


    I2C1_LastStatus = status;

    return status;
}


//==========================================================
// Escritura de un único byte
//==========================================================

I2C_Status I2C1_WriteSingleByte(
    uint8_t address7,
    uint8_t data
)
{
    return I2C1_Write(
        address7,
        &data,
        1u
    );
}


//==========================================================
// Lectura de uno o varios bytes
//==========================================================

I2C_Status I2C1_Read(
    uint8_t address7,
    uint8_t *data,
    uint8_t length
)
{
    uint8_t index;
    I2C_Status status;


    //======================================================
    // Validación de parámetros
    //======================================================

    if ((address7 > 0x7Fu) ||
        (data == NULL) ||
        (length == 0u))
    {
        I2C1_LastStatus = I2C_ERROR_INVALID_PARAMETER;

        return I2C1_LastStatus;
    }


    //======================================================
    // Espera del bus
    //======================================================

    status = I2C1_WaitBusFree();

    if (status != I2C_OK)
    {
        I2C1_LastStatus = status;

        I2C1_ResetModule();

        return status;
    }


    /*
     * Limpia las banderas de la operación anterior.
     */
    I2C1_ClearFlags();


    /*
     * La operación finalizará automáticamente con Stop.
     */
    I2C1CON0bits.RSEN = 0u;


    /*
     * ACKCNT = 1:
     *
     * Cuando I2C1CNT llegue a cero, el maestro enviará
     * NACK para indicar que termina la lectura.
     */
    I2C1CON1bits.ACKCNT = 1u;


    /*
     * Número de bytes que serán recibidos.
     */
    I2C1CNT = length;


    /*
     * Dirección de siete bits más R/W = 1.
     *
     * Ejemplo para el DS1307:
     *
     * 0x68 << 1 = 0xD0.
     * 0xD0 | 1  = 0xD1.
     */
    I2C1ADB1 = (uint8_t)(
        (address7 << 1) | 0x01u
    );


    /*
     * Genera la condición Start.
     */
    I2C1CON0bits.S = 1u;


    //======================================================
    // Recepción de los bytes
    //======================================================

    for (index = 0u; index < length; index++)
    {
        /*
         * Espera hasta que exista un byte disponible.
         */
        status = I2C1_WaitRxBufferFull();

        if (status != I2C_OK)
        {
            /*
             * El módulo también se recupera ante un NACK
             * producido por un dispositivo desconectado.
             */
            I2C1_LastStatus = status;

            I2C1_ResetModule();

            return status;
        }

        /*
         * Leer I2C1RXB libera el búfer para que el
         * periférico pueda recibir el siguiente byte.
         */
        data[index] = I2C1RXB;
    }


    //======================================================
    // Finalización
    //======================================================

    /*
     * Cuando el contador llega a cero:
     *
     * 1. El maestro envía NACK.
     * 2. El hardware genera la condición Stop.
     */
    status = I2C1_WaitStopRead();


    if (status != I2C_OK)
    {
        I2C1_ResetModule();
    }
    else
    {
        I2C1_ClearFlags();
    }


    I2C1_LastStatus = status;

    return status;
}