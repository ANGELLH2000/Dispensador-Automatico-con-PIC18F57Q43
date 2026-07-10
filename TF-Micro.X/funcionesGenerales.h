#ifndef FUNCIONES_GENERALES_H
#define FUNCIONES_GENERALES_H

#include <xc.h>
#include <stdint.h>
/*==============================================================================
 * PROTOTIPOS DE FUNCIONES
 *============================================================================*/

void config_perifericos(void);
void config_perifericos_sensores(void);
void SubProceso_CondicionesIniciales(void);
void SubProceso_DispersacionVerificacion(void);
void SubProceso_MenuLCD(void);
void PantallaGeneral(void);
void DataEEPROM(uint8_t data_memoria[40]);
void configuro(void);

#endif