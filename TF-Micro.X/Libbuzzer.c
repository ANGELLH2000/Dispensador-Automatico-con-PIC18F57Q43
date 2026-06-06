#include "Libbuzzer.h"


/*
 * Coloca el pin del buzzer en estado alto.
 *
 * Esta función es interna de la librería. Modifica únicamente el bit
 * correspondiente al buzzer, sin alterar los demás bits del puerto.
 */
static void Buzzer_PinHigh(Buzzer *buzzer)
{
    *(buzzer->lat) = (uint8_t)(*(buzzer->lat) | buzzer->pin_mask);
}


/*
 * Coloca el pin del buzzer en estado bajo.
 *
 * Esta función es interna de la librería. Modifica únicamente el bit
 * correspondiente al buzzer, sin alterar los demás bits del puerto.
 */
static void Buzzer_PinLow(Buzzer *buzzer)
{
    *(buzzer->lat) = (uint8_t)(*(buzzer->lat) & (uint8_t)(~buzzer->pin_mask));
}


/*
 * Genera un retardo variable en microsegundos.
 *
 * Las funciones __delay_us() de XC8 suelen requerir valores constantes o
 * simples. Para permitir retardos variables, se repite un retardo de 1 us
 * la cantidad de veces indicada.
 *
 * Parámetros:
 *
 * time_us:
 * Tiempo aproximado de retardo en microsegundos.
 */
static void Buzzer_DelayUs(uint32_t time_us)
{
    while(time_us > 0)
    {
        __delay_us(1);
        time_us--;
    }
}


/*
 * Inicializa el pin del buzzer.
 *
 * La función guarda los registros asociados al puerto, configura el pin como
 * salida digital y deja el buzzer apagado al inicio.
 */
void Buzzer_Init(Buzzer *buzzer,
                 volatile uint8_t *lat,
                 volatile uint8_t *tris,
                 volatile uint8_t *ansel,
                 uint8_t pin_mask)
{
    /*
     * Guarda las direcciones de los registros del puerto.
     */
    buzzer->lat = lat;
    buzzer->tris = tris;
    buzzer->ansel = ansel;

    /*
     * Guarda la máscara del pin utilizado por el buzzer.
     */
    buzzer->pin_mask = pin_mask;

    /*
     * Configura el pin seleccionado como digital.
     *
     * En ANSEL:
     * 0 = digital.
     * 1 = analógico.
     */
    *(buzzer->ansel) = (uint8_t)(*(buzzer->ansel) & (uint8_t)(~buzzer->pin_mask));

    /*
     * Configura el pin seleccionado como salida.
     *
     * En TRIS:
     * 0 = salida.
     * 1 = entrada.
     */
    *(buzzer->tris) = (uint8_t)(*(buzzer->tris) & (uint8_t)(~buzzer->pin_mask));

    /*
     * Estado inicial del buzzer apagado.
     */
    Buzzer_PinLow(buzzer);
}


/*
 * Genera un tono en el buzzer.
 *
 * El tono se genera alternando el pin del buzzer entre estado alto y bajo.
 * La frecuencia define el periodo de la señal, mientras que el duty define
 * qué porcentaje del periodo permanece en alto.
 *
 * Ejemplo:
 *
 * freq = 1000 Hz
 * periodo = 1000000 / 1000 = 1000 us
 *
 * duty = 50
 * tiempo alto = 500 us
 * tiempo bajo = 500 us
 */
void Buzzer_Tone(Buzzer *buzzer,
                 uint16_t freq,
                 uint16_t time_ms,
                 uint8_t duty)
{
    uint32_t period_us;
    uint32_t high_time_us;
    uint32_t low_time_us;
    uint32_t cycles;
    uint32_t i;

    /*
     * Evita división entre cero.
     */
    if(freq == 0)
    {
        return;
    }

    /*
     * Limita el ciclo de trabajo a un rango seguro.
     *
     * Un duty de 0 produce silencio.
     * Un duty de 100 mantiene el pin casi todo el tiempo en alto.
     */
    if(duty > 100)
    {
        duty = 100;
    }

    /*
     * Calcula el periodo total de la señal en microsegundos.
     */
    period_us = 1000000UL / freq;

    /*
     * Calcula el tiempo en alto y en bajo según el ciclo de trabajo.
     */
    high_time_us = (period_us * duty) / 100UL;
    low_time_us = period_us - high_time_us;

    /*
     * Calcula la cantidad de ciclos necesarios para aproximar la duración
     * total del tono.
     */
    cycles = ((uint32_t)time_ms * 1000UL) / period_us;

    /*
     * Genera la señal cuadrada durante la cantidad de ciclos calculada.
     */
    for(i = 0; i < cycles; i++)
    {
        if(high_time_us > 0)
        {
            Buzzer_PinHigh(buzzer);
            Buzzer_DelayUs(high_time_us);
        }

        if(low_time_us > 0)
        {
            Buzzer_PinLow(buzzer);
            Buzzer_DelayUs(low_time_us);
        }
    }

    /*
     * Asegura que el buzzer quede apagado al finalizar el tono.
     */
    Buzzer_PinLow(buzzer);
}


/*
 * Apaga el buzzer.
 *
 * Coloca el pin del buzzer en estado bajo.
 */
void Buzzer_Off(Buzzer *buzzer)
{
    Buzzer_PinLow(buzzer);
}


/*
 * Reproduce un sonido corto de click de botón.
 *
 * Uso recomendado:
 * Confirmar que el sistema registró una pulsación.
 *
 * Característica:
 * Sonido muy breve y discreto.
 */
void Buzzer_ButtonClick(Buzzer *buzzer)
{
    Buzzer_Tone(buzzer, 2500, 20, 50);
    Buzzer_Off(buzzer);
}


/*
 * Reproduce un sonido corto de confirmación final.
 *
 * Uso recomendado:
 * Confirmar que una opción, selección o acción puntual fue aceptada.
 *
 * Característica:
 * Secuencia breve ascendente.
 */
void Buzzer_FinalCorrectClick(Buzzer *buzzer)
{
    Buzzer_Tone(buzzer, NOTE_E5, 45, 50);
    __delay_ms(15);

    Buzzer_Tone(buzzer, NOTE_G5, 65, 50);
    __delay_ms(20);

    Buzzer_Off(buzzer);
}


/*
 * Reproduce un sonido de proceso correcto.
 *
 * Uso recomendado:
 * Indicar que la entrega de pastillas fue completada correctamente.
 *
 * Característica:
 * Secuencia ascendente, breve y clara.
 */
void Buzzer_CorrectSound(Buzzer *buzzer)
{
    Buzzer_Tone(buzzer, NOTE_C5, 80, 50);
    __delay_ms(20);

    Buzzer_Tone(buzzer, NOTE_E5, 90, 50);
    __delay_ms(20);

    Buzzer_Tone(buzzer, NOTE_G5, 120, 50);
    __delay_ms(20);

    Buzzer_Tone(buzzer, NOTE_C6, 180, 50);
    __delay_ms(40);

    Buzzer_Off(buzzer);
}


/*
 * Reproduce un sonido de advertencia.
 *
 * Uso recomendado:
 * Indicar una condición que requiere atención sin representar
 * necesariamente un error crítico.
 *
 * Ejemplos:
 * - Nivel bajo de pastillas.
 * - Usuario no retiró la dosis.
 * - Espera de confirmación.
 * - Mantenimiento próximo.
 *
 * Característica:
 * Tres pulsos medios repetidos.
 */
void Buzzer_WarningSound(Buzzer *buzzer)
{
    Buzzer_Tone(buzzer, NOTE_A5, 90, 50);
    __delay_ms(80);

    Buzzer_Tone(buzzer, NOTE_A5, 90, 50);
    __delay_ms(80);

    Buzzer_Tone(buzzer, NOTE_A5, 90, 50);
    __delay_ms(100);

    Buzzer_Off(buzzer);
}


/*
 * Reproduce un sonido de error.
 *
 * Uso recomendado:
 * Indicar una condición incorrecta, fallo en la entrega, atasco mecánico,
 * error de sensor o condición inválida.
 *
 * Característica:
 * Secuencia descendente y grave, fácil de diferenciar del sonido correcto.
 */
void Buzzer_ErrorSound(Buzzer *buzzer)
{
    Buzzer_Tone(buzzer, NOTE_G4, 160, 60);
    __delay_ms(40);

    Buzzer_Tone(buzzer, NOTE_E4, 180, 60);
    __delay_ms(40);

    Buzzer_Tone(buzzer, NOTE_C4, 250, 70);
    __delay_ms(60);

    Buzzer_Off(buzzer);
}