#include "cabecera.h"
#include "Libbuzzer.h"

Buzzer buzzer1;

void config(void)
{
    /*
     * Configuración del oscilador interno.
     * Debe coincidir con _XTAL_FREQ definido en cabecera.h.
     */
    OSCCON1 = 0x60;
    OSCFRQ = 0x07;
    OSCEN  = 0x40;
}

void main(void)
{
    config();

    /*
     * Inicializa el buzzer en RA0.
     *
     * Puerto usado: A.
     * Pin usado: RA0.
     * Máscara RA0: 0x01.
     */
    Buzzer_Init(&buzzer1, &LATA, &TRISA, &ANSELA, 0x01);

    while(1)
    {
        /*
         * Click simple de botón.
         */
        Buzzer_ButtonClick(&buzzer1);
        __delay_ms(1000);

        /*
         * Confirmación corta final.
         */
        Buzzer_FinalCorrectClick(&buzzer1);
        __delay_ms(1000);

        /*
         * Proceso completado correctamente.
         */
        Buzzer_CorrectSound(&buzzer1);
        __delay_ms(1000);

        /*
         * Advertencia del sistema.
         */
        Buzzer_WarningSound(&buzzer1);
        __delay_ms(1000);

        /*
         * Error del sistema.
         */
        Buzzer_ErrorSound(&buzzer1);
        __delay_ms(1000);
    }
}