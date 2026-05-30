#include <xc.h>
#include "cabecera.h"
#define _XTAL_FREQ 48000000UL  //unsigned long Se le especifca al preograma para que sepa a cuanto trabaja

void config(void) {
    //conf mod de oscilador
    OSCCON1 = 0x60;
    OSCFRQ  = 0x07;
    OSCEN = 0x40;
    //Entradas y Salidas
    
}
void main(void) {
    config();
    
    return;
}
