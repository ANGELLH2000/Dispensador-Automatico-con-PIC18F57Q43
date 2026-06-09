#ifndef CNY70_H
#define CNY70_H

#include <xc.h>
#include <stdint.h>

//====================================================
// MODOS DE DETECCIÓN
//====================================================

#define CNY70_ACTIVE_LOW   0
#define CNY70_ACTIVE_HIGH  1

//====================================================
// CANAL INTERNO DE DESCARGA
//====================================================
// Según tabla ADPCH del PIC18F27/47/57Q43:
// VSS / Analog Ground = 0b111011 = 0x3B
#define CNY70_ADC_DISCHARGE_CHANNEL  0x3B

//====================================================
// ESTRUCTURA DEL SENSOR CNY70
//====================================================
typedef struct
{
    volatile unsigned char *tris_reg;
    volatile unsigned char *ansel_reg;

    unsigned char pin_mask;
    unsigned char adc_channel;

} CNY70;

//====================================================
// PROTOTIPOS
//====================================================

void CNY70_ADC_Init_FOSC64(void);

void CNY70_Init(CNY70 *sensor,
                volatile unsigned char *tris,
                volatile unsigned char *ansel,
                unsigned char pin_mask,
                unsigned char adc_channel);

uint16_t CNY70_Read(CNY70 *sensor);

uint8_t CNY70_IsActive(CNY70 *sensor,
                       uint16_t threshold,
                       uint8_t mode);

void CNY70_ResetChannel(void);

#endif