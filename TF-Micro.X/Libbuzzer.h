#ifndef LIBBUZZER_H
#define LIBBUZZER_H

#include "cabecera.h"

/*
 * Librería para control de buzzer pasivo mediante generación de tonos.
 *
 * Esta librería permite generar sonidos utilizando un pin digital del
 * PIC18F57Q43. Está orientada al uso de buzzers pasivos, ya que estos
 * permiten generar diferentes tonos mediante una señal cuadrada.
 *
 * La librería permite:
 *
 * - Seleccionar el puerto y pin usado por el buzzer.
 * - Configurar el pin como salida digital.
 * - Generar tonos con frecuencia, duración y ciclo de trabajo.
 * - Reproducir sonidos predefinidos para eventos del sistema.
 *
 * Esta versión está pensada para un sistema de entrega o dispensación
 * automática de pastillas, por lo que incluye sonidos para:
 *
 * - Click de botón.
 * - Confirmación final corta.
 * - Entrega correcta.
 * - Advertencia.
 * - Error.
 *
 * La frecuencia del sistema debe estar definida en cabecera.h mediante
 * _XTAL_FREQ, ya que la librería utiliza __delay_us() y __delay_ms().
 */


/*
 * Frecuencias musicales utilizadas por los sonidos del sistema.
 *
 * Los valores están expresados en Hz.
 */
#define NOTE_C4   262
#define NOTE_D4   294
#define NOTE_E4   330
#define NOTE_G4   392
#define NOTE_A4   440

#define NOTE_C5   523
#define NOTE_D5   587
#define NOTE_E5   659
#define NOTE_F5   698
#define NOTE_G5   784
#define NOTE_A5   880
#define NOTE_B5   988

#define NOTE_C6   1047


/*
 * Estructura de configuración para un buzzer.
 *
 * Cada buzzer que se desee controlar debe tener una variable propia
 * de tipo Buzzer.
 *
 * Campos:
 *
 * lat:
 * Puntero al registro LAT del puerto utilizado.
 * Se usa para escribir el estado lógico del pin.
 *
 * tris:
 * Puntero al registro TRIS del puerto utilizado.
 * Se usa para configurar el pin como entrada o salida.
 *
 * ansel:
 * Puntero al registro ANSEL del puerto utilizado.
 * Se usa para configurar el pin como analógico o digital.
 *
 * pin_mask:
 * Máscara del pin donde está conectado el buzzer.
 *
 * Ejemplos:
 *
 * RA0 -> 0x01
 * RA1 -> 0x02
 * RA2 -> 0x04
 * RA3 -> 0x08
 *
 * RB0 -> 0x01
 * RB1 -> 0x02
 * RB2 -> 0x04
 * RB3 -> 0x08
 */
typedef struct
{
    volatile uint8_t *lat;
    volatile uint8_t *tris;
    volatile uint8_t *ansel;

    uint8_t pin_mask;

} Buzzer;


/*
 * Inicializa el pin del buzzer.
 *
 * Esta función configura el pin seleccionado como salida digital y lo deja
 * inicialmente en estado bajo.
 *
 * Parámetros:
 *
 * buzzer:
 * Dirección de la estructura Buzzer asociada al buzzer.
 *
 * lat:
 * Dirección del registro LAT del puerto utilizado.
 * Ejemplo: &LATA
 *
 * tris:
 * Dirección del registro TRIS del puerto utilizado.
 * Ejemplo: &TRISA
 *
 * ansel:
 * Dirección del registro ANSEL del puerto utilizado.
 * Ejemplo: &ANSELA
 *
 * pin_mask:
 * Máscara del pin donde está conectado el buzzer.
 *
 * Ejemplo de uso:
 *
 * Buzzer buzzer1;
 * Buzzer_Init(&buzzer1, &LATA, &TRISA, &ANSELA, 0x01);
 *
 * En este ejemplo se usa el pin RA0.
 */
void Buzzer_Init(Buzzer *buzzer,
                 volatile uint8_t *lat,
                 volatile uint8_t *tris,
                 volatile uint8_t *ansel,
                 uint8_t pin_mask);


/*
 * Genera un tono en el buzzer.
 *
 * El tono se genera alternando el pin del buzzer entre estado alto y bajo.
 *
 * Parámetros:
 *
 * buzzer:
 * Dirección de la estructura Buzzer asociada al buzzer.
 *
 * freq:
 * Frecuencia del tono en Hz.
 *
 * time_ms:
 * Duración del tono en milisegundos.
 *
 * duty:
 * Ciclo de trabajo del tono, expresado en porcentaje.
 *
 * Valores recomendados:
 *
 * 50:
 * Señal cuadrada equilibrada.
 *
 * 60 a 75:
 * Sonidos de mayor presencia, útiles para error o advertencia.
 */
void Buzzer_Tone(Buzzer *buzzer,
                 uint16_t freq,
                 uint16_t time_ms,
                 uint8_t duty);


/*
 * Apaga el buzzer.
 *
 * Coloca el pin asociado al buzzer en estado bajo.
 */
void Buzzer_Off(Buzzer *buzzer);


/*
 * Reproduce un sonido corto de click de botón.
 *
 * Uso recomendado:
 * Confirmar que el sistema registró una pulsación.
 */
void Buzzer_ButtonClick(Buzzer *buzzer);


/*
 * Reproduce un sonido corto de confirmación final.
 *
 * Uso recomendado:
 * Confirmar que una selección, configuración o acción puntual fue aceptada.
 */
void Buzzer_FinalCorrectClick(Buzzer *buzzer);


/*
 * Reproduce un sonido de proceso correcto.
 *
 * Uso recomendado:
 * Indicar que la entrega de pastillas fue completada correctamente.
 */
void Buzzer_CorrectSound(Buzzer *buzzer);


/*
 * Reproduce un sonido de advertencia.
 *
 * Uso recomendado:
 * Indicar una condición que requiere atención, pero que no representa
 * necesariamente un error crítico.
 *
 * Ejemplos:
 * - Usuario no retiró la dosis.
 * - Nivel bajo de pastillas.
 * - Mantenimiento próximo.
 * - Espera de confirmación.
 */
void Buzzer_WarningSound(Buzzer *buzzer);


/*
 * Reproduce un sonido de error.
 *
 * Uso recomendado:
 * Indicar fallo en la entrega, error de sensor, atasco mecánico o
 * condición inválida.
 */
void Buzzer_ErrorSound(Buzzer *buzzer);

#endif