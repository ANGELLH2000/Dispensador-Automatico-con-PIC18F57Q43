#ifndef WS2812B_H
#define WS2812B_H

#include "cabecera.h"

/*
 * Cantidad de LEDs de la tira WS2812B.
 *
 * Para la prueba actual se utilizan 3 LEDs.
 */
#define WS2812B_NUM_LEDS  3


/*
 * Pin de datos de la tira WS2812B.
 *
 * En esta configuración se utiliza específicamente el pin RD0
 * del PIC18F57Q43.
 *
 * LATD0:
 * Registro de salida del pin RD0.
 *
 * TRISD0:
 * Configuración de dirección del pin RD0.
 * 0 = salida
 * 1 = entrada
 *
 * ANSELD0:
 * Configuración analógica/digital del pin RD0.
 * 0 = digital
 * 1 = analógico
 */
#define WS2812B_LAT      LATDbits.LATD0
#define WS2812B_TRIS     TRISDbits.TRISD0
#define WS2812B_ANSEL    ANSELDbits.ANSELD0


void WS2812B_Init(void);
void WS2812B_SetPixel(uint8_t led, uint8_t red, uint8_t green, uint8_t blue);
void WS2812B_SetAll(uint8_t red, uint8_t green, uint8_t blue);
void WS2812B_Clear(void);
void WS2812B_Show(void);

#endif