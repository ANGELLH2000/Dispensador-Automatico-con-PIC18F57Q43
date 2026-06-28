#ifndef FUNCIONES_GENERALES_H
#define FUNCIONES_GENERALES_H

#include <xc.h>
#include <stdint.h>
/*==============================================================================
 * PROTOTIPOS DE FUNCIONES
 *============================================================================*/

void config_perifericos(void);
void config_perifericos_sensores(void);
void verificar_condiciones_iniciales(void);
void sistem_error(const char *mensaje);
void configuro(void);

#endif