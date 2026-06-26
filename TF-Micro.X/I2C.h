#ifndef I2C_H
#define I2C_H

/**
 * @file I2C.h
 * @brief Controlador genérico del periférico I2C1.
 *
 * Microcontrolador objetivo: PIC18F57Q43.
 * Compilador: MPLAB XC8.
 *
 * Configuración utilizada:
 *
 * RC3 -> SCL
 * RC4 -> SDA
 * Maestro con direccionamiento de 7 bits.
 * Velocidad del bus: 100 kHz.
 */

#include <stdint.h>


//==========================================================
// Estados de las operaciones I2C
//==========================================================

/**
 * @brief Resultados posibles de una operación I2C.
 */
typedef enum
{
    I2C_OK = 0,

    /*
     * El dispositivo no respondió con ACK.
     *
     * Puede deberse a:
     *
     * - Dirección incorrecta.
     * - Dispositivo desconectado.
     * - Dispositivo sin alimentación.
     */
    I2C_ERROR_NACK,

    /*
     * Se detectó una colisión en el bus.
     */
    I2C_ERROR_COLLISION,

    /*
     * Se produjo el timeout de hardware del bus.
     */
    I2C_ERROR_BUS_TIMEOUT,

    /*
     * Se agotó el contador de seguridad por software.
     */
    I2C_ERROR_SOFTWARE_TIMEOUT,

    /*
     * Se recibió una dirección, longitud o puntero inválido.
     */
    I2C_ERROR_INVALID_PARAMETER

} I2C_Status;


//==========================================================
// Variable de estado
//==========================================================

/**
 * @brief Último estado producido por el periférico I2C1.
 */
extern volatile I2C_Status I2C1_LastStatus;


//==========================================================
// Inicialización
//==========================================================

/**
 * @brief Inicializa el periférico I2C1.
 *
 * Configuración:
 *
 * - Maestro de 7 bits.
 * - RC3 como SCL.
 * - RC4 como SDA.
 * - Open-drain.
 * - 100 kHz.
 * - Funcionamiento mediante polling.
 */
void I2C1_Init(void);


//==========================================================
// Escritura
//==========================================================

/**
 * @brief Escribe un bloque de bytes en un dispositivo I2C.
 *
 * @param address7 Dirección de 7 bits sin incluir R/W.
 * @param data Arreglo que contiene los bytes que serán enviados.
 * @param length Número de bytes que deben transmitirse.
 *
 * @return Estado final de la operación.
 *
 * Ejemplo:
 *
 * uint8_t datos[2] = {0x00, 0x25};
 * I2C1_Write(0x68, datos, 2);
 */
I2C_Status I2C1_Write(
    uint8_t address7,
    const uint8_t *data,
    uint8_t length
);


/**
 * @brief Escribe un único byte en un dispositivo I2C.
 *
 * @param address7 Dirección de 7 bits sin incluir R/W.
 * @param data Byte que será transmitido.
 *
 * @return Estado final de la operación.
 */
I2C_Status I2C1_WriteSingleByte(
    uint8_t address7,
    uint8_t data
);


//==========================================================
// Lectura
//==========================================================

/**
 * @brief Lee uno o más bytes desde un dispositivo I2C.
 *
 * @param address7 Dirección de 7 bits sin incluir R/W.
 * @param data Arreglo donde se guardarán los bytes recibidos.
 * @param length Número de bytes que deben recibirse.
 *
 * @return Estado final de la operación.
 *
 * El maestro envía automáticamente un NACK después del último
 * byte para finalizar correctamente la lectura.
 */
I2C_Status I2C1_Read(
    uint8_t address7,
    uint8_t *data,
    uint8_t length
);


#endif /* I2C_H */
