#include "motor_paso.h"

/*
 * Secuencia de paso completo para motor paso a paso de 4 señales.
 *
 * Orden lógico de señales:
 *
 * IN1 IN2 IN3 IN4
 *
 * Secuencia:
 *
 * 1100
 * 0110
 * 0011
 * 1001
 *
 * Esta secuencia activa dos fases al mismo tiempo. Esto permite obtener
 * mayor torque en comparación con una secuencia de una sola fase activa.
 *
 * Dependiendo del orden físico de conexión entre el driver y el motor,
 * puede ser necesario modificar el orden de esta secuencia.
 */
static const uint8_t full_step_sequence[4] = {
    0b1100,
    0b0110,
    0b0011,
    0b1001
};


/*
 * Escribe un valor de 4 bits en la sección del puerto asignada al motor.
 *
 * Esta función es interna de la librería. No se declara en el archivo .h
 * porque no debe ser llamada directamente desde el programa principal.
 *
 * La función solo modifica los cuatro bits asignados al motor. Los demás
 * bits del puerto se conservan sin cambios.
 *
 * Parámetros:
 *
 * motor:
 * Dirección de la estructura Stepper asociada al motor.
 *
 * value:
 * Valor de 4 bits que representa el estado de las salidas del motor.
 */
static void Stepper_Write(Stepper *motor, uint8_t value)
{
    uint8_t mask;
    uint8_t output_value;

    if(motor->nibble == STEPPER_LOW_NIBBLE)
    {
        /*
         * Uso de la parte baja del puerto:
         * bit 0, bit 1, bit 2 y bit 3.
         */
        mask = 0x0F;

        /*
         * Se asegura que solo se usen los cuatro bits menos significativos.
         */
        output_value = value & 0x0F;

        /*
         * Se conservan los bits 4 a 7 y se actualizan únicamente
         * los bits 0 a 3.
         */
        *(motor->lat) = (uint8_t)((*(motor->lat) & (uint8_t)(~mask)) | output_value);
    }
    else
    {
        /*
         * Uso de la parte alta del puerto:
         * bit 4, bit 5, bit 6 y bit 7.
         */
        mask = 0xF0;

        /*
         * Se desplaza el valor de 4 bits hacia la parte alta del puerto.
         */
        output_value = (uint8_t)((value & 0x0F) << 4);

        /*
         * Se conservan los bits 0 a 3 y se actualizan únicamente
         * los bits 4 a 7.
         */
        *(motor->lat) = (uint8_t)((*(motor->lat) & (uint8_t)(~mask)) | output_value);
    }
}


/*
 * Inicializa un motor paso a paso.
 *
 * La función recibe la estructura del motor, los registros del puerto a usar
 * y la sección del puerto seleccionada.
 *
 * Además, configura los cuatro bits seleccionados como salidas digitales
 * y los inicializa en estado bajo.
 */
void Stepper_Init(Stepper *motor,
                  volatile uint8_t *lat,
                  volatile uint8_t *tris,
                  volatile uint8_t *ansel,
                  uint8_t nibble)
{
    /*
     * Se almacenan las direcciones de los registros en la estructura
     * asociada al motor.
     */
    motor->lat = lat;
    motor->tris = tris;
    motor->ansel = ansel;

    /*
     * Se almacena la sección del puerto que utilizará el motor.
     */
    motor->nibble = nibble;

    /*
     * Se inicializa el índice de la secuencia de pasos.
     */
    motor->index = 0;

    if(motor->nibble == STEPPER_LOW_NIBBLE)
    {
        /*
         * Configuración de los bits 0, 1, 2 y 3 como salidas.
         *
         * En TRIS:
         * 0 = salida
         * 1 = entrada
         */
        *(motor->tris) &= 0xF0;

        /*
         * Configuración de los bits 0, 1, 2 y 3 como digitales.
         *
         * En ANSEL:
         * 0 = digital
         * 1 = analógico
         */
        *(motor->ansel) &= 0xF0;

        /*
         * Inicialización de las salidas en estado bajo.
         */
        *(motor->lat) &= 0xF0;
    }
    else
    {
        /*
         * Configuración de los bits 4, 5, 6 y 7 como salidas.
         */
        *(motor->tris) &= 0x0F;

        /*
         * Configuración de los bits 4, 5, 6 y 7 como digitales.
         */
        *(motor->ansel) &= 0x0F;

        /*
         * Inicialización de las salidas en estado bajo.
         */
        *(motor->lat) &= 0x0F;
    }
}


/*
 * Ejecuta un paso en sentido horario.
 *
 * La función escribe el estado actual de la secuencia y luego avanza
 * el índice a la siguiente posición.
 */
void Stepper_Step_CW(Stepper *motor)
{
    /*
     * Se escribe el estado actual de la secuencia en las salidas.
     */
    Stepper_Write(motor, full_step_sequence[motor->index]);

    /*
     * Se avanza a la siguiente posición de la secuencia.
     */
    motor->index++;

    /*
     * Si el índice supera la última posición, vuelve al inicio.
     */
    if(motor->index >= 4)
    {
        motor->index = 0;
    }
}


/*
 * Ejecuta un paso en sentido antihorario.
 *
 * La función escribe el estado actual de la secuencia y luego retrocede
 * el índice a la posición anterior.
 */
void Stepper_Step_CCW(Stepper *motor)
{
    /*
     * Se escribe el estado actual de la secuencia en las salidas.
     */
    Stepper_Write(motor, full_step_sequence[motor->index]);

    /*
     * Se retrocede dentro de la secuencia.
     */
    if(motor->index == 0)
    {
        motor->index = 3;
    }
    else
    {
        motor->index--;
    }
}


/*
 * Ejecuta una cantidad determinada de pasos en sentido horario.
 *
 * El parámetro delay_ms define el tiempo de espera entre cada paso.
 * Este valor controla la velocidad de giro del motor.
 */
void Stepper_Move_CW(Stepper *motor, uint16_t steps, uint16_t delay_ms)
{
    uint16_t i;
    uint16_t j;

    for(i = 0; i < steps; i++)
    {
        /*
         * Se ejecuta un paso en sentido horario.
         */
        Stepper_Step_CW(motor);

        /*
         * Retardo entre pasos.
         *
         * Se implementa mediante repeticiones de 1 ms para permitir
         * el uso de una variable como tiempo de retardo.
         */
        for(j = 0; j < delay_ms; j++)
        {
            __delay_ms(1);
        }
    }
}


/*
 * Ejecuta una cantidad determinada de pasos en sentido antihorario.
 *
 * El funcionamiento es equivalente a Stepper_Move_CW(), pero recorriendo
 * la secuencia de pasos en sentido inverso.
 */
void Stepper_Move_CCW(Stepper *motor, uint16_t steps, uint16_t delay_ms)
{
    uint16_t i;
    uint16_t j;

    for(i = 0; i < steps; i++)
    {
        /*
         * Se ejecuta un paso en sentido antihorario.
         */
        Stepper_Step_CCW(motor);

        /*
         * Retardo entre pasos.
         */
        for(j = 0; j < delay_ms; j++)
        {
            __delay_ms(1);
        }
    }
}


/*
 * Ejecuta una vuelta completa del eje externo del motor 28BYJ-48
 * en sentido horario.
 */
void Stepper_fullTurn_CW(Stepper *motor)
{
    Stepper_Move_CW(motor,
                    STEPPER_STEPS_PER_REV_28BYJ48,
                    STEPPER_DEFAULT_DELAY_MS);
    Stepper_Off(motor);
}


/*
 * Ejecuta una vuelta completa del eje externo del motor 28BYJ-48
 * en sentido antihorario.
 */
void Stepper_fullTurn_CCW(Stepper *motor)
{
    Stepper_Move_CCW(motor,
                     STEPPER_STEPS_PER_REV_28BYJ48,
                     STEPPER_DEFAULT_DELAY_MS);
    Stepper_Off(motor);
}


/*
 * Desactiva las cuatro salidas asignadas al motor.
 *
 * Esta función coloca en cero los cuatro bits utilizados por el motor.
 *
 * Al desactivar las salidas, el motor puede perder torque de retención.
 */
void Stepper_Off(Stepper *motor)
{
    Stepper_Write(motor, 0x00);
}