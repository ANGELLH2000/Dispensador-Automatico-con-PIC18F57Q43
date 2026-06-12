#ifndef EPROM_DFM_H
#define EPROM_DFM_H

/**
 * Esta librería permite leer, escribir y actualizar datos en la memoria
 * no volátil DFM del PIC18F57Q43.
 *
 * Aunque comúnmente se le puede llamar EEPROM de datos, en este
 * microcontrolador la hoja de datos la define como DFM
 * (Data Flash Memory).
 *
 * La DFM permite almacenar información que debe conservarse incluso
 * después de apagar o reiniciar el microcontrolador.
 *
 * Ejemplos de uso:
 * - Guardar configuraciones del sistema.
 * - Guardar contadores.
 * - Guardar estados de operación.
 * - Guardar valores de calibración.
 */

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * EEPROM_DFM_BASE
 * Dirección base de la memoria DFM en el PIC18F57Q43.
 *
 * La memoria DFM se encuentra mapeada a partir de la dirección 0x380000.
 *
 * Esta dirección no se usa directamente desde el programa principal.
 * En su lugar, el usuario trabaja con direcciones lógicas desde 0 hasta 1023.
 *
 * Ejemplo:
 *
 * Dirección lógica 0  -> Dirección real 0x380000
 * Dirección lógica 1  -> Dirección real 0x380001
 * Dirección lógica 10 -> Dirección real 0x38000A
 */
#define EEPROM_DFM_BASE      0x380000UL

/**
 * EEPROM_DFM_SIZE
 * Tamaño total de la memoria DFM disponible.
 *
 * El PIC18F57Q43 dispone de 1024 bytes de memoria DFM.
 *
 * Por lo tanto, las direcciones válidas para el usuario van desde:
 *
 * 0 hasta 1023
 *
 * o en hexadecimal:
 *
 * 0x000 hasta 0x3FF
 */
#define EEPROM_DFM_SIZE      1024u

/**
 * Escribe un byte en la memoria DFM.
 *
 * Esta función escribe directamente un dato de 8 bits en una dirección
 * lógica de la memoria DFM.
 *
 * La dirección recibida no es la dirección absoluta 0x380000.
 * Es una dirección lógica relativa al inicio de la DFM.
 *
 * Ejemplo:
 *
 * EEPROM_WriteByte(0, 25);
 *
 * En ese caso, la función escribirá el valor 25 en la dirección real:
 *
 * 0x380000 + 0
 *
 * @param address Dirección lógica de la DFM.
 *                Rango válido: 0 a 1023.
 *
 * @param data Dato de 8 bits que se desea guardar.
 *
 * @note Esta función escribe siempre, aunque el valor nuevo sea igual
 *       al valor ya almacenado. Para reducir desgaste de la memoria,
 *       se recomienda usar EEPROM_UpdateByte().
 */
void EEPROM_WriteByte(uint16_t address, uint8_t data);

/**
 * @brief Lee un byte desde la memoria DFM.
 *
 * Esta función lee el contenido de una dirección lógica de la memoria DFM
 * y devuelve el dato almacenado.
 *
 * Ejemplo:
 *
 * uint8_t dato = EEPROM_ReadByte(0);
 *
 * En ese caso, la función leerá desde la dirección real:
 *
 * 0x380000 + 0
 *
 * @param address Dirección lógica de la DFM.
 *                Rango válido: 0 a 1023.
 *
 * @return Dato de 8 bits almacenado en la dirección indicada.
 *
 * @note Si la memoria no ha sido escrita previamente, normalmente puede
 *       leerse como 0xFF.
 */
uint8_t EEPROM_ReadByte(uint16_t address);

/**
 * Actualiza un byte en la memoria DFM solo si el dato cambió.
 *
 * Esta función primero lee el dato almacenado en la dirección indicada.
 * Luego compara ese dato con el nuevo valor.
 *
 * Si ambos valores son diferentes, realiza la escritura.
 * Si ambos valores son iguales, no escribe nada.
 *
 * Esto ayuda a reducir el desgaste de la memoria no volátil.
 *
 * Ejemplo:
 *
 * EEPROM_UpdateByte(0, 25);
 *
 * Si en la dirección 0 ya estaba guardado el valor 25, no se realiza
 * ninguna escritura.
 *
 * @param address Dirección lógica de la DFM.
 *                Rango válido: 0 a 1023.
 *
 * @param data Nuevo dato de 8 bits que se desea guardar.
 *
 * Esta función es recomendada para guardar configuraciones,
 * valores modificados por botones, contadores o parámetros
 * que no necesitan escribirse continuamente.
 */
void EEPROM_UpdateByte(uint16_t address, uint8_t data);

#endif