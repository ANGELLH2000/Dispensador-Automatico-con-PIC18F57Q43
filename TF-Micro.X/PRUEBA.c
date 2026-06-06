#include <xc.h>
#include <pic18f57q43.h>
#include "cabecera.h"
#include "Libbuzzer.h"

#define _XTAL_FREQ 64000000UL


void main(void){
    //================================================
    // OSCILADOR INTERNO = 64 MHz
    //================================================
    OSCCON1 = 0x60;
    OSCFRQ  = 0x07;
    OSCEN   = 0x40;
    
    buzzer_pin();
    
    while(1){
    correct_sound( );
    __delay_ms(2000);
    error_sound( );
    
    __delay_ms(2000);
    mantainance_sound( );
    __delay_ms(2000);
    
    button_sound( );
    __delay_ms(2000);
    
     mario_theme();
    __delay_ms(2000);
    
    }
    
}

