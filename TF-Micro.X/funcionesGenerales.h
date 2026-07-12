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
void PantallaSensores(void);
void DataEEPROM(uint8_t data_memoria[40]);
void configuro(void);
void Funcion_AgregarHorario(uint8_t hora,uint8_t min, uint8_t pastillero,uint8_t horario);
void Funcion_AgregarPastillas(uint8_t  pastillero_selecionado , uint8_t cantidad_a_sumar);
void Sensores(uint16_t lectura_sensores[4]);

#endif