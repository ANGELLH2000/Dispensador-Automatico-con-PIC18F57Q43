#include "EPROM_DFM.h"

//====================================================
// FUNCION INTERNA: CARGA DE DIRECCION DFM
//====================================================
// Esta funcion convierte una direccion logica de usuario
// en una direccion real dentro del mapa de memoria del PIC.
//
// El usuario trabaja con direcciones simples:
//      0, 1, 2, 3 ... 1023
//
// Pero la memoria DFM del PIC18F57Q43 inicia realmente en:
//      0x380000
//
// Por ejemplo:
//      address = 0   -> direccion real = 0x380000
//      address = 1   -> direccion real = 0x380001
//      address = 10  -> direccion real = 0x38000A
//
// La direccion real se carga en los registros NVMADR,
// que son utilizados por el modulo NVM para leer o escribir
// en memoria no volatil.
//====================================================
static void EEPROM_SetAddress(uint16_t address)
{
    uint32_t real_address = EEPROM_DFM_BASE + address;

    // Parte alta de la direccion real.
    // Para la DFM del PIC18F57Q43, normalmente sera 0x38.
    NVMADRU = (uint8_t)((real_address >> 16) & 0xFF);

    // Parte media de la direccion real.
    NVMADRH = (uint8_t)((real_address >> 8) & 0xFF);

    // Parte baja de la direccion real.
    NVMADRL = (uint8_t)(real_address & 0xFF);
}

//====================================================
// FUNCION: EEPROM_ReadByte
//====================================================
// Lee un byte desde la memoria DFM.
//
// Parametros:
//      address -> direccion logica de memoria.
//                 Rango permitido: 0 a 1023.
//
// Retorna:
//      Dato leido desde la memoria DFM.
//
// Si la direccion esta fuera del rango permitido,
// la funcion retorna 0xFF como valor de seguridad.
//
// Nota:
//      La lectura no requiere secuencia de desbloqueo.
//      Solo se carga la direccion, se selecciona el comando
//      de lectura y se inicia la operacion con el bit GO.
//====================================================
uint8_t EEPROM_ReadByte(uint16_t address)
{
    // Verifica que la direccion no exceda el tamaño
    // disponible de la memoria DFM.
    if (address >= EEPROM_DFM_SIZE)
    {
        return 0xFF;
    }

    // Carga la direccion real 0x38xxxx en los registros NVMADR.
    EEPROM_SetAddress(address);

    // Selecciona el comando de lectura.
    // NVMCMD = 000 corresponde a operacion de lectura.
    NVMCON1bits.NVMCMD = 0b000;

    // Inicia la operacion de lectura.
    NVMCON0bits.GO = 1;

    // Espera hasta que el modulo NVM termine la lectura.
    while (NVMCON0bits.GO);

    // El dato leido queda almacenado en NVMDATL.
    return NVMDATL;
}

//====================================================
// FUNCION: EEPROM_WriteByte
//====================================================
// Escribe un byte directamente en la memoria DFM.
//
// Parametros:
//      address -> direccion logica de memoria.
//                 Rango permitido: 0 a 1023.
//
//      data    -> dato de 8 bits que se desea guardar.
//
// Importante:
//      Esta funcion escribe siempre el dato, aunque el valor
//      nuevo sea igual al que ya estaba guardado.
//
//      Para reducir desgaste de la memoria, se recomienda usar
//      EEPROM_UpdateByte() cuando no sea necesario forzar
//      la escritura.
//
// Nota:
//      La escritura en memoria no volatil requiere una secuencia
//      obligatoria de desbloqueo mediante NVMLOCK.
//====================================================
void EEPROM_WriteByte(uint16_t address, uint8_t data)
{
    // Verifica que la direccion no exceda el tamaño
    // disponible de la memoria DFM.
    if (address >= EEPROM_DFM_SIZE)
    {
        return;
    }

    // Carga la direccion real 0x38xxxx en los registros NVMADR.
    EEPROM_SetAddress(address);

    // Carga el dato que se desea escribir.
    // Para escritura de un byte se utiliza NVMDATL.
    NVMDATL = data;

    // Selecciona el comando de escritura en DFM.
    NVMCON1bits.NVMCMD = 0b011;

    // Deshabilita interrupciones durante la secuencia critica.
    // Esto evita que una interrupcion interrumpa el desbloqueo
    // de escritura de la memoria no volatil.
    INTCON0bits.GIE = 0;

    // Secuencia obligatoria de desbloqueo.
    // Esta secuencia protege la memoria contra escrituras accidentales.
    NVMLOCK = 0x55;
    NVMLOCK = 0xAA;

    // Inicia la operacion de escritura.
    NVMCON0bits.GO = 1;

    // Espera hasta que el modulo NVM termine la escritura.
    while (NVMCON0bits.GO);

    // Habilita nuevamente las interrupciones globales.
    INTCON0bits.GIE = 1;

    // Limpia el comando NVM para evitar operaciones no deseadas.
    NVMCON1bits.NVMCMD = 0b000;
}

//====================================================
// FUNCION: EEPROM_UpdateByte
//====================================================
// Actualiza un byte en la memoria DFM solo si el dato cambio.
//
// Parametros:
//      address -> direccion logica de memoria.
//                 Rango permitido: 0 a 1023.
//
//      data    -> nuevo dato que se desea guardar.
//
// Funcionamiento:
//      1. Lee el dato actual guardado en la direccion indicada.
//      2. Compara el dato actual con el nuevo dato.
//      3. Si son diferentes, escribe el nuevo valor.
//      4. Si son iguales, no realiza escritura.
//
// Ventaja:
//      Reduce el desgaste de la memoria DFM, porque evita
//      escrituras innecesarias.
//====================================================
void EEPROM_UpdateByte(uint16_t address, uint8_t data)
{
    uint8_t old_data = EEPROM_ReadByte(address);

    // Solo escribe si el dato nuevo es diferente
    // al dato que ya estaba almacenado.
    if (old_data != data)
    {
        EEPROM_WriteByte(address, data);
    }
}