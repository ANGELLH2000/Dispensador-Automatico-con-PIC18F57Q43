#include "CNY70.h"

//====================================================
// VARIABLE INTERNA DE CONTROL DE CANAL
//====================================================
// Esta variable guarda cuál fue el último canal leído.
// Sirve para saber si el ADC cambió de canal o no.
static unsigned char cny70_last_channel = 0xFF;


//====================================================
// INICIALIZACIÓN GENERAL DEL ADC
//====================================================
void CNY70_ADC_Init_FOSC64(void)
{
    /*
     * Esta función configura solamente el ADC.
     *
     * NO configura:
     * - OSCCON1
     * - OSCFRQ
     * - OSCEN
     * - _XTAL_FREQ
     *
     * El oscilador general del PIC debe configurarse
     * en cabecera.h o en main.c.
     */

    ADREF = 0x00;         // VREF+ = VDD, VREF- = VSS

    /*
     * ADCLK = 0b011111 = 31
     *
     * Fórmula:
     * FADC = FOSC / (2 * (ADCLK + 1))
     *
     * Si FOSC = 64 MHz:
     * FADC = 64 MHz / (2 * (31 + 1))
     * FADC = 64 MHz / 64
     * FADC = 1 MHz
     */
    ADCLK = 0b000111;     // FOSC/64

    /*
     * ADCON0 = 0x84
     *
     * ADC habilitado.
     * Resultado justificado a la derecha.
     * Reloj ADC derivado de FOSC, no ADCRC.
     */
    ADCON0 = 0x84;

    /*
     * Se fuerza a que la primera lectura haga descarga,
     * porque todavía no hay un canal anterior válido.
     */
    cny70_last_channel = 0xFF;
}


//====================================================
// INICIALIZACIÓN DE UN SENSOR CNY70
//====================================================
void CNY70_Init(CNY70 *sensor,
                volatile unsigned char *tris,
                volatile unsigned char *ansel,
                unsigned char pin_mask,
                unsigned char adc_channel)
{
    /*
     * Ejemplo:
     *
     * CNY70_Init(&sensor1, &TRISA, &ANSELA, 0x08, 0x03);
     *
     * &TRISA  -> registro TRIS del puerto A
     * &ANSELA -> registro ANSEL del puerto A
     * 0x08    -> máscara del pin RA3
     * 0x03    -> canal AN3
     */

    sensor->tris_reg = tris;
    sensor->ansel_reg = ansel;
    sensor->pin_mask = pin_mask;
    sensor->adc_channel = adc_channel;

    /*
     * Pin como entrada.
     * TRIS = 1 -> entrada
     */
    *(sensor->tris_reg) |= sensor->pin_mask;

    /*
     * Pin como analógico.
     * ANSEL = 1 -> analógico
     */
    *(sensor->ansel_reg) |= sensor->pin_mask;
}


//====================================================
// REINICIAR CANAL ACTUAL
//====================================================
void CNY70_ResetChannel(void)
{
    /*
     * Esta función se puede usar si desde otra parte del programa
     * se modificó ADPCH manualmente.
     *
     * Al llamar esta función, la próxima lectura de CNY70_Read()
     * volverá a hacer descarga del capacitor.
     */
    cny70_last_channel = 0xFF;
}


//====================================================
// LECTURA SIMPLE DEL SENSOR
//====================================================
uint16_t CNY70_Read(CNY70 *sensor)
{
    uint16_t result = 0;

    /*
     * Solo se hace descarga si:
     *
     * - Es la primera lectura.
     * - O el canal actual es diferente al canal anterior.
     *
     * Si se lee repetidamente el mismo sensor, no se descarga
     * el capacitor en cada lectura.
     */
    if(cny70_last_channel != sensor->adc_channel)
    {
        //================================================
        // 1. Seleccionar canal interno VSS para descarga
        //================================================
        ADPCH = CNY70_ADC_DISCHARGE_CHANNEL;

        //================================================
        // 2. Conversión falsa de descarga
        //================================================
        ADCON0bits.GO_nDONE = 1;

        while(ADCON0bits.GO_nDONE == 1)
        {
            // Espera fin de conversión de descarga
        }

        /*
         * El resultado de esta conversión se descarta.
         * Su objetivo es descargar el capacitor interno
         * del ADC hacia VSS.
         */

        //================================================
        // 3. Seleccionar canal real del sensor
        //================================================
        ADPCH = sensor->adc_channel;

        //================================================
        // 4. Conversión falsa después del cambio de canal
        //================================================
        ADCON0bits.GO_nDONE = 1;

        while(ADCON0bits.GO_nDONE == 1)
        {
            // Espera fin de conversión falsa
        }

        /*
         * Esta conversión también se descarta.
         * Sirve para que el ADC se estabilice luego de cambiar
         * desde VSS hacia el canal real.
         */

        //================================================
        // 5. Guardar el canal actual
        //================================================
        cny70_last_channel = sensor->adc_channel;
    }

    //====================================================
    // Lectura válida
    //====================================================
    ADCON0bits.GO_nDONE = 1;

    while(ADCON0bits.GO_nDONE == 1)
    {
        // Espera fin de conversión válida
    }

    result = ((uint16_t)ADRESH << 8) | ADRESL;

    return result;
}


//====================================================
// DETECCIÓN POR UMBRAL
//====================================================
uint8_t CNY70_IsActive(CNY70 *sensor,
                       uint16_t threshold,
                       uint8_t mode)
{
    uint16_t value = 0;

    value = CNY70_Read(sensor);

    if(mode == CNY70_ACTIVE_LOW)
    {
        /*
         * Activo en bajo:
         * Se activa cuando el ADC baja.
         *
         * Ejemplo:
         * Barrera IR:
         * - Haz libre: ADC alto
         * - Haz bloqueado: ADC bajo
         */
        if(value <= threshold)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        /*
         * Activo en alto:
         * Se activa cuando el ADC sube.
         *
         * Ejemplo:
         * CNY70 reflectivo:
         * - Fondo oscuro: ADC bajo
         * - Objeto claro: ADC alto
         */
        if(value >= threshold)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}