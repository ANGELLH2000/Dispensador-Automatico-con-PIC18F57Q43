#include <xc.h>
#include <stdint.h>
#include <stdio.h>
#include "motor_paso.h"
#include "ws2812b.h"
#include "Libbuzzer.h"
#include "keypad4x4.h"
#include "I2C.h"
#include "LCD_I2C.h"
#include "DS1307.h"
#include "CNY70.h"
#include "irsensor.h"
#include "EPROM_DFM.h"
#include "funcionesGenerales.h"

/*==============================================================================
 * OBJETOS DE LOS PERIFÉRICOS
 *============================================================================*/

/* Motores paso a paso */
Stepper motor1;
Stepper motor2;
Stepper motor3;
Stepper motor4;

/* Tira de LEDs WS2812B */
LED_WS2812B tira1;

/* Buzzer */
Buzzer buzzer1;

/* Teclado matricial 4x4 */
Keypad teclado;
char tecla;

/* Sensores reflectivos CNY70 */
CNY70 sensor1;
CNY70 sensor2;
CNY70 sensor3;
CNY70 sensor4;

/* Valores obtenidos de los sensores CNY70 */
uint16_t valor_S1 = 0;
uint16_t valor_S2 = 0;
uint16_t valor_S3 = 0;
uint16_t valor_S4 = 0;

/* Sensor infrarrojo digital */
IRsensor sensor_ir;

/* Estructura para almacenar la fecha y hora del RTC */
DS1307_DateTime fechaHora;

/* Estado de las operaciones realizadas mediante I2C */
I2C_Status estado;

/* Variable para la lectura de la Memoria */
uint8_t dato_memoria;

/*==============================================================================
 * PROTOTIPOS DE FUNCIONES INTERNAS
 *============================================================================*/

static void config_motores(void);
static void config_leds(void);
static void config_buzzer(void);
static void config_keypad(void);
static void config_cny70(void);
static void config_ir(void);
static void config_i2c_lcd(void);
static void SubProceso_ModificarHorario();
static void SubProceso_AgregarHorario();
static void SubProceso_VerHorarios();
static void SubProceso_RegistrarPastillas();
static void SubProceso_DispensacionVerificacion();
static void SubProceso_ManejoErrores(char *mensaje,uint8_t nivel_error);
static void Dispensar(uint8_t pastillas_por_dispensar[6],uint8_t cantidad_horarios_para_dispersar);
static void Detallar_Horarios(uint8_t numero,uint16_t hora,uint16_t minuto,uint16_t tipo);
static void Detallar_CantPastillas(uint8_t  tecla);
static void Guardar_CantPastillas(uint8_t  pastillero_selecionado , uint8_t cantidad_a_sumar);
bool RecorrerEEPROM();

/*==============================================================================
 * CONFIGURACIÓN DE LOS MOTORES PASO A PASO
 *============================================================================*/

static void config_motores(void)
{
    /*
     * Motores conectados al puerto A:
     *
     * Motor 1: RA0 a RA3, nibble bajo.
     * Motor 2: RA4 a RA7, nibble alto.
     */
    Stepper_Init(&motor1, &LATA, &TRISA, &ANSELA, STEPPER_LOW_NIBBLE);
    Stepper_Init(&motor2, &LATA, &TRISA, &ANSELA, STEPPER_HIGH_NIBBLE);

    /*
     * Motores conectados al puerto B:
     *
     * Motor 3: RB0 a RB3, nibble bajo.
     * Motor 4: RB4 a RB7, nibble alto.
     */
    Stepper_Init(&motor3, &LATB, &TRISB, &ANSELB, STEPPER_LOW_NIBBLE);
    Stepper_Init(&motor4, &LATB, &TRISB, &ANSELB, STEPPER_HIGH_NIBBLE);
}

/*==============================================================================
 * CONFIGURACIÓN DE LA TIRA LED
 *============================================================================*/

static void config_leds(void)
{
    /*
     * Inicializa una tira WS2812B compuesta por dos LEDs.
     * El pin de comunicación se define en ws2812b.h.
     */
    WS2812B_Init(&tira1, 1);
}

/*==============================================================================
 * CONFIGURACIÓN DEL BUZZER
 *============================================================================*/

static void config_buzzer(void)
{
    /*
     * Buzzer conectado al pin RC1.
     *
     * 0x02 = 0b00000010.
     */
    Buzzer_Init(&buzzer1, &LATC, &TRISC, &ANSELC, 0x02);
    Buzzer_Off(&buzzer1);
}

/*==============================================================================
 * CONFIGURACIÓN DEL TECLADO MATRICIAL
 *============================================================================*/

static void config_keypad(void)
{
    /*
     * Teclado matricial 4x4 conectado al puerto D.
     *
     * RD0 a RD3: filas.
     * RD4 a RD7: columnas.
     */
    Keypad_Init(&teclado, &PORTD, &LATD, &TRISD, &ANSELD, &WPUD);
}

/*==============================================================================
 * CONFIGURACIÓN DE LOS SENSORES CNY70
 *============================================================================*/

static void config_cny70(void)
{
    /*
     * Configura el conversor ADC utilizando FOSC/64
     * como reloj de conversión.
     */
    CNY70_ADC_Init_FOSC64();

    /*
     * Sensor 1: RF0 / ANF0.
     * Sensor 2: RF1 / ANF1.
     * Sensor 3: RF2 / ANF2.
     * Sensor 4: RF3 / ANF3.
     */
    CNY70_Init(&sensor1, &TRISF, &ANSELF, 0x01, 0x28);
    CNY70_Init(&sensor2, &TRISF, &ANSELF, 0x02, 0x29);
    CNY70_Init(&sensor3, &TRISF, &ANSELF, 0x04, 0x2A);
    CNY70_Init(&sensor4, &TRISF, &ANSELF, 0x08, 0x2B);
}

/*==============================================================================
 * CONFIGURACIÓN DEL SENSOR INFRARROJO
 *============================================================================*/

static void config_ir(void)
{
    /*
     * Sensor infrarrojo digital conectado al pin RF4.
     *
     * 0x10 = 0b00010000.
     */
    IRSensor_Init(&sensor_ir, &PORTF, &TRISF, &ANSELF, 0x10);
}

/*==============================================================================
 * CONFIGURACIÓN DEL BUS I2C Y DEL LCD
 *============================================================================*/

static void config_i2c_lcd(void)
{
    /*
     * Inicializa el módulo I2C1.
     *
     * El LCD y el RTC DS1307 comparten el mismo bus I2C.
     */
    I2C1_Init();

    /*
     * Establece la dirección del expansor PCF8574
     * utilizado por la pantalla LCD.
     */
    estado = LCD_I2C_SetAddress(PCF8574_7);

    /*
     * Inicializa la pantalla LCD.
     */
    LCD_I2C_Init();

    /*
     * Limpia la pantalla después de su inicialización.
     */
    LCD_I2C_Clear();
}

/*==============================================================================
 * CONFIGURACIÓN GENERAL DE LOS PERIFÉRICOS
 *============================================================================*/

void config_perifericos(void)
{
    /*
     * Inicializa todos los periféricos utilizados
     * por el sistema.
     */
    
    //Boton de Congif
    TRISEbits.TRISE0=1;
    WPUEbits.WPUE0=1;
    ANSELEbits.ANSELE0=0;
    
    config_motores();
    config_leds();
    config_buzzer();
    config_keypad();
    config_cny70();
    config_ir();
    config_i2c_lcd();
    //Configuracion de memoria
    if(PORTEbits.RE0 == 0){ 
        LCD_I2C_SetCursor(2, 0);
        LCD_I2C_WriteString("BTN. Config - ON");
        __delay_ms(500);
        for (uint8_t x=0;x<39;x++)
        {
            EEPROM_WriteByte(x, 0);
        }
        EEPROM_WriteByte(1, 1);//Cant Horarios
        EEPROM_WriteByte(3, 4);//Cantidad de cOMPARTIMINETOS
        EEPROM_WriteByte(2, 4);//Cantidad total de pastillas
        EEPROM_WriteByte(5, 4);//Cantidad de pastillas en el cmpartimento1
        EEPROM_WriteByte(9, 13);//hora
        EEPROM_WriteByte(10, 47);//min
        EEPROM_WriteByte(13, 1);//PASTILLERO
        
        
    }else{
        LCD_I2C_SetCursor(2, 0);
        LCD_I2C_WriteString("BTN. Config - OFF");
        if(!RecorrerEEPROM())
            SubProceso_ManejoErrores("Mem. Desconfigurada",3);
        __delay_ms(500);
    }
        
    
}



/*
 * ============================================================================
 * FUNCIÓN: SubProceso_DispensacionVerificacion
 * ============================================================================
 * Verifica constantemente si la hora actual del sistema coincide con algún 
 * horario programado en la memoria para activar la dispensación de medicamentos.
 *
 * Flujo:
 * 1. Lee la fecha y hora actual del módulo RTC DS1307.
 * 2. Verifica la integridad y conexión del reloj en tiempo real.
 * 3. Recorre la cantidad de horarios actualmente activos en la EEPROM.
 * 4. Compara la hora y minuto actuales con los almacenados.
 * 5. Si hay coincidencia y no se ha dispensado, encola el pastillero.
 * 6. Si no hay coincidencia, resetea las banderas de dispensación.
 * 7. Ejecuta la función principal de dispensación si hay pastillas encoladas.
 * ============================================================================
 */
void SubProceso_DispensacionVerificacion(void)
{
    /*
     * Variables locales
     */
    uint8_t pastillas_por_dispensar[6] = {};
    uint8_t cantidad_horarios_para_dispersar = 0;
    uint8_t cant_horarios = EEPROM_ReadByte(1);
    bool dispensar = false;

    /*----------------------------------------------------------------------
     * Lectura y validación del reloj (RTC)
     *----------------------------------------------------------------------*/
    
    estado = DS1307_ReadDateTime(&fechaHora);

    /* Validar pérdida de conexión o corrupción de datos en el reloj */
    if ((estado != I2C_OK) || (fechaHora.clock_running == 0) || (fechaHora.data_valid == 0))
    {
        SubProceso_ManejoErrores("RTC sin conexion", 1);
    }
       
    /*----------------------------------------------------------------------
     * Búsqueda de coincidencias de horarios programados
     *----------------------------------------------------------------------*/
    
    for (uint8_t x = 1; x <= cant_horarios; x++)
    {
        /* * EEPROM: 
         * 9 + ((x-1)*5) = Dirección de la hora 
         * 10 + ((x-1)*5) = Dirección de los minutos 
         */
        if (EEPROM_ReadByte(9 + ((x - 1) * 5)) == fechaHora.hours && EEPROM_ReadByte(10 + ((x - 1) * 5)) == fechaHora.minutes)
        {
            /* * Bandera de dispensación (11 + offset). 
             * Si es 0, significa que todavía no se ha dispensado en este minuto.
             */
            if (EEPROM_ReadByte(11 + ((x - 1) * 5)) == 0)
            {
                /* Guardar el número del pastillero a dispensar (13 + offset) */
                pastillas_por_dispensar[x - 1] = EEPROM_ReadByte(13 + ((x - 1) * 5));
                cantidad_horarios_para_dispersar++;
                dispensar = true;
            }
            
        }
        else
        {
            /* * Si la hora no coincide (ya pasó el minuto de activación), 
             * nos aseguramos de resetear las banderas de "ya dispensado" a 0
             * para todos los slots (11, 16, 21, 26, 31, 36) para que estén 
             * listos para el día siguiente.
             */
            EEPROM_UpdateByte(11, 0);
            EEPROM_UpdateByte(16, 0);
            EEPROM_UpdateByte(21, 0);
            EEPROM_UpdateByte(26, 0);
            EEPROM_UpdateByte(31, 0);
            EEPROM_UpdateByte(36, 0);
        }  
    }

    /*----------------------------------------------------------------------
     * Ejecución de la dispensación física
     *----------------------------------------------------------------------*/
    
    /* Si se encoló al menos un pastillero, llamar a la rutina principal */
    LCD_I2C_SetCursor(2, 0);
    LCD_I2C_WriteString("aQUI");
    if (dispensar)
        Dispensar(pastillas_por_dispensar, cantidad_horarios_para_dispersar);
    
}

void Dispensar(uint8_t pastillas_por_dispensar[6],uint8_t cantidad_horarios_para_dispersar)
{
    bool error=false;
    bool dispensado;
    uint16_t lectura_sensor;
    
    //Sensor del vaso
    if(IRSensor_ReadActiveHigh(&sensor_ir))
    {
        SubProceso_ManejoErrores("Colocar el Vaso",2);
        error=true;
    }
    
    //Cantidad Necesaria
    for (uint8_t x=0; x<cantidad_horarios_para_dispersar;x++)
    {
        if(EEPROM_ReadByte(4+(pastillas_por_dispensar[x]))==0)
        {   
            char mensaje[20];

            sprintf(mensaje,"Pastillero %u Vacio", pastillas_por_dispensar[x]);
            SubProceso_ManejoErrores(mensaje,2);
            error=true;
        }
            
    }
    if(!error)
    {
        LCD_I2C_Clear();
        LCD_I2C_SetCursor(2, 0);
        LCD_I2C_WriteString("! HORA DE LA DOSIS !");
        Buzzer_Off(&buzzer1);
        for (uint8_t x=0; x<cantidad_horarios_para_dispersar;x++)
        {
            dispensado = false;
            
            LCD_I2C_SetCursor(3, 0);
            LCD_I2C_WriteString("Entregando: Past. ");
            LCD_I2C_WriteUInt8(pastillas_por_dispensar[x],1);
            
            uint8_t intentos=4;
            while(1)
            {
                               
                for(uint16_t y=0; y<2048;y++)
                {
                    switch (pastillas_por_dispensar[x])
                    {

                        case 1:
                            lectura_sensor=CNY70_Read(&sensor1);
                            Stepper_Step_CW(&motor1);
                            break;
                        case 2:
                            lectura_sensor=CNY70_Read(&sensor2);
                            Stepper_Step_CW(&motor2);
                            break;
                        case 3:
                            lectura_sensor=CNY70_Read(&sensor3);
                            Stepper_Step_CW(&motor3);
                            break;
                        case 4:
                            lectura_sensor=CNY70_Read(&sensor4);
                            Stepper_Step_CW(&motor4);
                            break;
                        /* Se recibió un número */
                        default:
                            break;

                    }
                    LCD_I2C_SetCursor(4, 0);
                    LCD_I2C_WriteInt(lectura_sensor);
                    if(lectura_sensor>1000)
                    {
                        Stepper_Off(&motor1);
                        Stepper_Off(&motor2);
                        Stepper_Off(&motor3);
                        Stepper_Off(&motor4);
                        dispensado = true;
                        break;
                    }
                    
                }
                if (dispensado)
                {
                    //Disminur cantida de pastillas
                    EEPROM_UpdateByte(4+(pastillas_por_dispensar[x]),EEPROM_ReadByte(4+(pastillas_por_dispensar[x]))-1);
                    EEPROM_UpdateByte(11+(x*5),1);
                    Buzzer_FinalCorrectClick(&buzzer1);
                    WS2812B_RGB(&tira1,0,200,0);
                    __delay_ms(500);
                    WS2812B_Clear(&tira1);
                    break;
                    
                }
                
                intentos--;
                Buzzer_WarningSound(&buzzer1);
                WS2812B_RGB(&tira1,240,200,0);
                __delay_ms(500);
                WS2812B_Clear(&tira1);
                if(intentos==0)
                {
                    SubProceso_ManejoErrores("No se pudo dispensar",3);
                    break;
                }
                
                
            }
            
            


        }
    }
    
}
/*
 * ============================================================================
 * FUNCIÓN: PantallaGeneral
 * ============================================================================
 * Muestra la pantalla principal de reposo del sistema. Actualiza constantemente 
 * la hora y espera la interacción del usuario para acceder al menú principal.
 *
 * Flujo:
 * 1. Inicia un bucle infinito de monitoreo.
 * 2. Verifica si existe alguna dispensación de medicamentos pendiente.
 * 3. Lee la fecha y hora actual del módulo RTC DS1307.
 * 4. Maneja posibles errores de conexión o pérdida de datos del RTC.
 * 5. Actualiza la pantalla LCD con el estado general y la hora actual.
 * 6. Monitorea el teclado de forma no bloqueante buscando la tecla '#'.
 * ============================================================================
 */
void PantallaGeneral(void)
{
    
        /*----------------------------------------------------------------------
         * Monitoreo de dispensación y lectura del RTC.
         *----------------------------------------------------------------------*/
        
        SubProceso_DispensacionVerificacion();
        LCD_I2C_SetCursor(2, 0);
        LCD_I2C_WriteString("Sistema en operacion");
        estado = DS1307_ReadDateTime(&fechaHora);

        /* Validación de estado de la comunicación y datos del RTC */
        if ((estado != I2C_OK) || (fechaHora.clock_running == 0) || (fechaHora.data_valid == 0))
        {
            SubProceso_ManejoErrores("RTC sin conexion", 1);
        }

        /*----------------------------------------------------------------------
         * Actualización de la interfaz LCD.
         *----------------------------------------------------------------------*/

        /*
                  12:30:45
            Sistema en operacion
            
            --- [#] VER MENU ---
        */

        LCD_I2C_SetCursor(2, 0);
        LCD_I2C_WriteString("Sistema en operacion");
        
        LCD_I2C_SetCursor(4, 0);
        LCD_I2C_WriteString("--- [#] VER MENU ---");
        
        LCD_I2C_SetCursor(3, 0);
        LCD_I2C_ClearFile(); /* Limpia la fila 3 para evitar residuos visuales */
        
        /* Impresión de la hora en formato HH:MM:SS centrada en la fila 1 */
        LCD_I2C_SetCursor(1, 6);
        LCD_I2C_WriteUInt8(fechaHora.hours, 2);
        LCD_I2C_WriteString(":"); 
        
        LCD_I2C_SetCursor(1, 9);
        LCD_I2C_WriteUInt8(fechaHora.minutes, 2);
        LCD_I2C_WriteString(":"); 
        
        LCD_I2C_SetCursor(1, 12);
        LCD_I2C_WriteUInt8(fechaHora.seconds, 2);
        
        /* Retardo para la tasa de refresco visual de la pantalla */
        __delay_ms(250);
        
        /*----------------------------------------------------------------------
         * Monitoreo del teclado.
         *----------------------------------------------------------------------*/
        
        while (1)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);

            switch (tecla)
            {
                /* Ingresar al menú de opciones */
                case '#':
                    
                    Buzzer_CorrectSound(&buzzer1);
                    SubProceso_MenuLCD();
                    return;

                /* Teclas ignoradas y estado inactivo (NO_KEY) */
                default:
                    
                    break;
            }
            
            /* * Se rompe el bucle de lectura inmediatamente para permitir 
             * que el bucle principal continúe y actualice el reloj cada 250ms.
             */
            break;
        }
           
}
/*==============================================================================
 * VERIFICACIÓN DE LAS CONDICIONES INICIALES
 *============================================================================*/
void SubProceso_CondicionesIniciales(void){
    
    //Bandera de Error=0
    
    /*
     * Verifica que la dirección del LCD se haya configurado
     * correctamente.
     */
    if (estado != I2C_OK)
        SubProceso_ManejoErrores("I2C sin conexion",1);
    
    //Lee la fecha y la hora almacenadas en el DS1307.
    estado = DS1307_ReadDateTime(&fechaHora);

    /*
     * Comprueba si el RTC respondió mediante el bus I2C.
     */
    if (estado != I2C_OK)
        SubProceso_ManejoErrores("RTC sin conexion",1);

    /*
     * clock_running igual a cero indica que el bit CH
     * está en uno y el oscilador se encuentra detenido.
     */
    if (fechaHora.clock_running == 0)
        SubProceso_ManejoErrores("RTC detenido",1);

    /*
     * data_valid igual a cero indica que la fecha o la hora
     * almacenada contiene valores fuera del rango permitido.
     */
    if (fechaHora.data_valid == 0)
        SubProceso_ManejoErrores("Mal Formato de Fecha",1);
    
    //EEPROM 
    //Recorrer todos los valores y verificar que no sea 0xFF
    if (RecorrerEEPROM()==false)
        SubProceso_ManejoErrores("EPROM no Configurada",1);
    
    if(EEPROM_ReadByte(1)==0)
        SubProceso_ManejoErrores("No hay Horarios",2);
    
    if(EEPROM_ReadByte(2)==0)
        SubProceso_ManejoErrores("No hay Pastillas",2);
    
    LCD_I2C_Clear();
    LCD_I2C_SetCursor(2, 0);
    LCD_I2C_WriteString("Sistema correcto");

    //Enciende la tira LED en color verde.
    WS2812B_RGB(&tira1, 0, 200, 0);
    __delay_ms(2000);
    WS2812B_Clear(&tira1);
    
    

}
//dEVUELVE TRUE CUNADO EL RECORRIDO SALIO SIN NING[UN 0XFF]
bool RecorrerEEPROM()
{
    for(uint8_t x=0;x<39;x++)
    {
        if (EEPROM_ReadByte(x)== 0xFF)
            return  false;
    }
    return true;
}

/*==============================================================================
 * SISTEMA DE ERROR
 *============================================================================*/

void SubProceso_ManejoErrores(char *mensaje,uint8_t nivel_error){
    /*
     * Limpia la pantalla y muestra el mensaje de error.
     */
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(2, 0);
    LCD_I2C_WriteString("Alerta en el Sistema");

    LCD_I2C_SetCursor(3, 0);
    LCD_I2C_WriteString(mensaje);
    
    //Critico - 1 
    //Alerta - 2
    switch (nivel_error)
        {
            case 1: 
                //Enciende la tira LED en color rojo.
                WS2812B_RGB(&tira1, 200, 0, 0);
                Buzzer_ErrorSound(&buzzer1);
                break;

            case 2:
                //Enciende la tira LED en color amarillo.
                WS2812B_RGB(&tira1, 250, 200, 0);
                Buzzer_WarningSound(&buzzer1);
                __delay_ms(2000);
                WS2812B_Clear(&tira1);
                return;
            case 3:
                //Enciende la tira LED en color amarillo.
                WS2812B_RGB(&tira1, 250, 200, 0);
                Buzzer_WarningSound(&buzzer1);
                LCD_I2C_SetCursor(4, 0);
                LCD_I2C_WriteString("Llamar Mantenimiento");
                while(1)
                {
                   __delay_ms(1000);
                }
                
                return;

            default:
                break;
               
        }
    __delay_ms(3000);
    
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(2, 0);
    LCD_I2C_WriteString("Reiniciar Sistema");
    LCD_I2C_SetCursor(3, 0);
    LCD_I2C_WriteString("[#]Resetear Sistema");
    
    while (1)
    {
        tecla = KEYPAD_NO_KEY;

        while (tecla == KEYPAD_NO_KEY)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);
        }

        switch (tecla)
        {
            
            case '#':
                Buzzer_ButtonClick(&buzzer1);
                asm("RESET");
                break;
            /* Se recibió un número */
            default:
                
                Buzzer_WarningSound(&buzzer1);
                tecla = KEYPAD_NO_KEY;

        }

        /* Salir únicamente cuando la selección sea válida */
        if (tecla != KEYPAD_NO_KEY)
            break;
    }
}
/******************************************************************************
 * Función: MostrarAnimacionCarga
 * ---------------------------------------------------------------------------
 * Muestra una animación de progreso utilizando 4 bloques sólidos.
 *
 * Parámetros:
 *      fila    -> Fila donde inicia la animación.
 *      columna -> Columna donde inicia la animación.
 ******************************************************************************/
void MostrarAnimacionCarga(unsigned char fila, unsigned char columna)
{
    LCD_I2C_CreateSolidPixel(0);
    LCD_I2C_SetCursor(fila, columna);

    for (unsigned char index = 0u; index < 4u; index++)
    {
        LCD_I2C_WriteChar((char)0u);
        __delay_ms(300);
    }
}


/******************************************************************************
 * Función: SubProceso_MenuLCD
 *----------------------------------------------------------------------------
 * Muestra el menú principal del sistema y espera a que el usuario seleccione
 * una opción mediante el teclado matricial.
 *
 * Opciones disponibles:
 *   1 -> Agregar horario
 *   2 -> Modificar horario
 *   3 -> Ver horarios
 *   4 -> Registrar pastillas
 *   * -> Resetear sistema
 *
 * Si se ingresa una opción inválida, el menú vuelve a mostrarse.
 ******************************************************************************/
void SubProceso_MenuLCD(void)
{
    /* Inicializar la tecla como "sin pulsar" */
    tecla = KEYPAD_NO_KEY;

    /* Mantener el menú activo hasta recibir una opción válida */
    while (tecla == KEYPAD_NO_KEY)
    {
        /*==============================================================
         * Pantalla de bienvenida
         *==============================================================*/
        LCD_I2C_Clear();

        LCD_I2C_SetCursor(2, 5);
        LCD_I2C_WriteString("Seleccionar");

        LCD_I2C_SetCursor(3, 3);
        LCD_I2C_WriteString("Opcion Deseada");

        __delay_ms(2000);


        /*==============================================================
         * Mostrar menú principal
         *==============================================================*/
        LCD_I2C_Clear();

        LCD_I2C_SetCursor(1, 0);
        LCD_I2C_WriteString("1.Agr.Hor  2.Mod.Hor");

        LCD_I2C_SetCursor(2, 0);
        LCD_I2C_WriteString("3.Ver horarios      ");

        LCD_I2C_SetCursor(3, 0);
        LCD_I2C_WriteString("4.Reg. pastillas    ");

        LCD_I2C_SetCursor(4, 0);
        LCD_I2C_WriteString("*.Salir del menu    ");


        /*==============================================================
         * Esperar la pulsación de una tecla
         * Tiempo máximo aproximado: 5 segundos
         * (500 iteraciones × 10 ms)
         *==============================================================*/
        for (unsigned int index = 0; index < 500; index++)
        {
            tecla = Keypad_Read(&teclado);

            if (tecla != KEYPAD_NO_KEY)
            {
                LCD_I2C_Clear();

                /*======================================================
                 * Procesar la opción seleccionada
                 *======================================================*/
                switch (tecla)
                {
                    case '1':
                        Buzzer_CorrectSound(&buzzer1);
                        LCD_I2C_SetCursor(1, 7);
                        LCD_I2C_WriteString("Opcion");
                        LCD_I2C_SetCursor(2, 2);
                        LCD_I2C_WriteString("Agregar Horario");
                        __delay_ms(2000);
                        SubProceso_AgregarHorario();
                        break;

                    case '2':
                        Buzzer_CorrectSound(&buzzer1);
                        LCD_I2C_SetCursor(1, 7);
                        LCD_I2C_WriteString("Opcion");
                        LCD_I2C_SetCursor(2, 1);
                        LCD_I2C_WriteString("Modificar Horario");
                        __delay_ms(2000);
                        SubProceso_ModificarHorario();
                        break;

                    case '3':
                        Buzzer_CorrectSound(&buzzer1);
                        LCD_I2C_SetCursor(1, 7);
                        LCD_I2C_WriteString("Opcion");
                        LCD_I2C_SetCursor(2, 4);
                        LCD_I2C_WriteString("Ver Horarios");
                        __delay_ms(2000);
                        SubProceso_VerHorarios();
                        break;

                    case '4':
                        Buzzer_CorrectSound(&buzzer1);
                        LCD_I2C_SetCursor(1, 7);
                        LCD_I2C_WriteString("Opcion");
                        LCD_I2C_SetCursor(2, 1);
                        LCD_I2C_WriteString("Registrar Pastilla");
                        __delay_ms(2000);
                        SubProceso_RegistrarPastillas();
                        break;

                    case '*':
                        Buzzer_CorrectSound(&buzzer1);
                        return;

                    default:
                        /* Opción inválida: informar al usuario y
                           volver a mostrar el menú. */
                        Buzzer_ErrorSound(&buzzer1);
                        LCD_I2C_SetCursor(2, 2);
                        LCD_I2C_WriteString("Opcion Invalida");
                        __delay_ms(500);

                        tecla = KEYPAD_NO_KEY;
                        break;
                }
            }

            /* Retardo para evitar lecturas excesivamente rápidas */
            __delay_ms(10);
        }
    }
}
/******************************************************************************
 * Función: SubProceso_VerHorarios
 *----------------------------------------------------------------------------
 * Permite visualizar los horarios almacenados en la memoria EEPROM.
 *
 * Flujo:
 *   1. Lee la cantidad de horarios registrados.
 *   2. Verifica que los datos almacenados sean válidos.
 *   3. Prepara la información para su visualización.
 *   4. Muestra los horarios en páginas de hasta 3 registros.
 *   5. Permite navegar entre páginas mediante el teclado.
 *
 * Controles:
 *   A -> Página anterior.
 *   B -> Página siguiente.
 *   * -> Salir.
 ******************************************************************************/
void SubProceso_VerHorarios(void)
{
    /*==============================================================
     * Leer cantidad de horarios registrados
     *==============================================================*/
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1,6);
    LCD_I2C_WriteString("Horarios");

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_WriteString("Leyendo Memoria");

    dato_memoria = EEPROM_ReadByte(1);

    MostrarAnimacionCarga(4,0);

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_ClearFile();

    LCD_I2C_SetCursor(3,2);
    LCD_I2C_WriteString("Extrayendo Datos");

    MostrarAnimacionCarga(4,4);

    /*==============================================================
     * Validar información almacenada
     *==============================================================*/
    if (dato_memoria == 0xFF || dato_memoria > 6)
    {
        MostrarAnimacionCarga(4,8);

        LCD_I2C_SetCursor(3,0);
        LCD_I2C_ClearFile();

        LCD_I2C_SetCursor(3,2);
        LCD_I2C_WriteString("Datos Corruptos");

        __delay_ms(1500);
        return;
    }

    if (dato_memoria == 0)
    {
        MostrarAnimacionCarga(4,8);

        LCD_I2C_SetCursor(3,0);
        LCD_I2C_ClearFile();

        LCD_I2C_SetCursor(3,0);
        LCD_I2C_WriteString("Sin Hor. Registrados");

        __delay_ms(1500);
        return;
    }

    /*==============================================================
     * Preparar información para mostrarla en pantalla
     *==============================================================*/
    LCD_I2C_SetCursor(3,0);
    LCD_I2C_ClearFile();

    LCD_I2C_SetCursor(3,2);
    LCD_I2C_WriteString("Preparando Datos");

    MostrarAnimacionCarga(4,8);
    MostrarAnimacionCarga(4,12);

    /*==============================================================
     * Variables de navegación
     *==============================================================*/
    uint8_t cant_horarios = dato_memoria;
    uint8_t pagina = 0;
    uint8_t paginas_max = (cant_horarios - 1) / 3;

    /*==============================================================
     * Navegación entre páginas
     *==============================================================*/
    while (1)
    {
        /*----------------------------------------------------------
         * Mostrar encabezado
         *----------------------------------------------------------*/
        LCD_I2C_Clear();

        LCD_I2C_SetCursor(1,0);
        LCD_I2C_WriteString("--- MIS HORARIOS ---");

        /*----------------------------------------------------------
         * Mostrar horarios de la página actual
         *----------------------------------------------------------*/
        uint8_t horarios_por_pagina;
        switch (pagina)
        {
            case 0:
                if(cant_horarios>3)
                {
                    horarios_por_pagina=3;
                }else
                {
                    horarios_por_pagina=cant_horarios;
                }
                
                for (uint8_t x=0;x<horarios_por_pagina;x++)
                {
                    LCD_I2C_SetCursor(x+2,0);
                    Detallar_Horarios(x+1,9+(5*x),10+(5*x),13+(5*x));
                }
                break;

            case 1:
                
                if(cant_horarios-3>3)
                {
                    horarios_por_pagina=3;
                }else
                {
                    horarios_por_pagina=cant_horarios-3;
                }
                
                for (uint8_t y=0;y<horarios_por_pagina;y++)
                {
                    LCD_I2C_SetCursor(y+2,0);
                    Detallar_Horarios(y+4,24+(5*y),25+(5*y),28+(5*y));
                }
                break;
        }

        /*----------------------------------------------------------
         * Mostrar controles de navegación
         *----------------------------------------------------------*/
        if (pagina > 0)
        {
            LCD_I2C_SetCursor(2,12);
            LCD_I2C_WriteString("[A]Subir");
        }

        if (pagina < paginas_max)
        {
            LCD_I2C_SetCursor(4,12);
            LCD_I2C_WriteString("[B]Bajar");
        }

        LCD_I2C_SetCursor(3,12);
        LCD_I2C_WriteString("[*]Salir");

        /*----------------------------------------------------------
         * Esperar una tecla del usuario
         *----------------------------------------------------------*/
        tecla = KEYPAD_NO_KEY;

        while (tecla == KEYPAD_NO_KEY)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);
        }

        /*----------------------------------------------------------
         * Procesar la tecla presionada
         *----------------------------------------------------------*/
        switch (tecla)
        {
            case 'A':

                Buzzer_ButtonClick(&buzzer1);

                if (pagina > 0)
                    pagina--;

                break;

            case 'B':

                Buzzer_ButtonClick(&buzzer1);

                if (pagina < paginas_max)
                    pagina++;

                break;

            case '*':

                Buzzer_ButtonClick(&buzzer1);
                return;

            default:

                /* Tecla sin función en esta pantalla */
                Buzzer_WarningSound(&buzzer1);
                break;
        }
    }
}
/*
 * ============================================================================
 * FUNCIÓN: SubProceso_ModificarHorario
 * ============================================================================
 * Permite modificar un horario de dispensación previamente registrado.
 *
 * Flujo:
 * 1. Verifica que existan horarios almacenados.
 * 2. Solicita seleccionar el horario a modificar.
 * 3. Solicita la nueva hora en formato 24 horas (HHMM).
 * 4. Solicita el nuevo número de pastillero (1-4).
 * 5. Espera confirmación final del usuario.
 * 6. Actualiza el horario en la EEPROM.
 * 7. Muestra un mensaje de confirmación.
 * ============================================================================
 */
void SubProceso_ModificarHorario(void)
{
    /*
     * Variables locales
     */
    uint8_t horario_selecionado;
    uint8_t pastillero_modificado;
    uint8_t hora_modificada[4] = {};
    
    /*----------------------------------------------------------------------
     * Verificar si existen horarios disponibles para modificar.
     *----------------------------------------------------------------------*/
    if (EEPROM_ReadByte(1) == 0)
    {
        LCD_I2C_Clear();
        LCD_I2C_SetCursor(2,3);
        LCD_I2C_WriteString("No hay Horarios");
        return;
    }
    
    /* Obtener la cantidad de horarios actualmente almacenados */
    dato_memoria = EEPROM_ReadByte(1);
        
    /*----------------------------------------------------------------------
     * Selección del horario a modificar.
     *----------------------------------------------------------------------*/

    /*
        - MODIFICAR HORARIO-
        Horarios activos: 03
        Elegir Horario: [ ]
        [*]Salir
    */
    
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1,0);
    LCD_I2C_WriteString("- MODIFICAR HORARIO-");

    LCD_I2C_SetCursor(2,0);
    LCD_I2C_WriteString("Horarios activos: ");
    LCD_I2C_WriteUInt8(dato_memoria,2);

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_WriteString("Elegir Horario: [ ]");
    
    LCD_I2C_SetCursor(4,0);
    LCD_I2C_WriteString("[*]Salir");
    
    while (1)
    {
        tecla = KEYPAD_NO_KEY;

        while (tecla == KEYPAD_NO_KEY)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);
        }

        switch (tecla)
        {
            /* Teclas no permitidas en la selección de horario */
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case '#':
                
                Buzzer_WarningSound(&buzzer1);
                tecla = KEYPAD_NO_KEY;
                break;

            /* Cancelar operación */
            case '*':

                Buzzer_ButtonClick(&buzzer1);
                return;

            /* Se recibió un número */
            default:
                
                Buzzer_ButtonClick(&buzzer1);

                horario_selecionado = tecla - '0';
                LCD_I2C_SetCursor(3,17);
                LCD_I2C_WriteChar(tecla);
                
                __delay_ms(500);
                
                /* Validar que el horario elegido exista (1 hasta dato_memoria) */
                if(horario_selecionado > dato_memoria || horario_selecionado == 0)
                {
                    Buzzer_WarningSound(&buzzer1);
                    tecla = KEYPAD_NO_KEY;
                }
                else
                {
                    break;
                }
        }

        /* Salir únicamente cuando la selección sea válida */
        if (tecla != KEYPAD_NO_KEY)
            break;
    }
    
    /* Confirmación del horario seleccionado */
    LCD_I2C_SetCursor(4,0);
    LCD_I2C_WriteString("[#]Selec. [*]Salir");
    
    while (1)
    {
        tecla = KEYPAD_NO_KEY;

        while (tecla == KEYPAD_NO_KEY)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);
        }

        switch (tecla)
        {
            case '#':

                Buzzer_ButtonClick(&buzzer1);
                __delay_ms(500);
                break;

            /* Cancelar operación */
            case '*':

                Buzzer_ButtonClick(&buzzer1);
                return;

            /* Cualquier otra tecla es inválida */
            default:

                Buzzer_WarningSound(&buzzer1);
                tecla = KEYPAD_NO_KEY;
        }

        if (tecla != KEYPAD_NO_KEY)
            break;
    }

    /*----------------------------------------------------------------------
     * Modificación de la hora.
     *----------------------------------------------------------------------*/

    /*
        - EDITAR HORARIO 1 -
        Hora actual:  08:30 
        Nueva hora :  00:00 
                    [*]Salir
    */
    
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1,0);
    LCD_I2C_WriteString("- EDITAR HORARIO _ -");
    LCD_I2C_SetCursor(1,17);
    LCD_I2C_WriteUInt8(horario_selecionado,1);
    
    LCD_I2C_SetCursor(2,0);
    LCD_I2C_WriteString("Hora actual:  __:__");
    
    LCD_I2C_SetCursor(2,14);
    dato_memoria = EEPROM_ReadByte(9+(5*(horario_selecionado-1))); // Hora actual
    LCD_I2C_WriteUInt8(dato_memoria,2);
    
    LCD_I2C_SetCursor(2,17);
    dato_memoria = EEPROM_ReadByte(10+(5*(horario_selecionado-1))); // Minutos actuales
    LCD_I2C_WriteUInt8(dato_memoria,2);

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_WriteString("Nueva hora :  xx:xx ");
    
    LCD_I2C_SetCursor(4,12);
    LCD_I2C_WriteString("[*]Salir");
    
    /*
     * Solicitar los cuatro dígitos: HHMM
     */
    for (uint8_t x = 0; x < 4; x++)
    {
        while (1)
        {
            tecla = KEYPAD_NO_KEY;

            while (tecla == KEYPAD_NO_KEY)
            {
                tecla = Keypad_Read(&teclado);
                __delay_ms(20);
            }

            switch (tecla)
            {
                /* Teclas no permitidas */
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case '#':

                    Buzzer_WarningSound(&buzzer1);
                    tecla = KEYPAD_NO_KEY;
                    break;

                /* Cancelar ingreso */
                case '*':

                    Buzzer_ButtonClick(&buzzer1);
                    return;

                /* Se recibió un número */
                default:

                    Buzzer_ButtonClick(&buzzer1);

                    /* Mostrar el dígito en pantalla */
                    if (x <= 1)
                        LCD_I2C_SetCursor(3, x + 14);
                    else
                        LCD_I2C_SetCursor(3, x + 15);

                    LCD_I2C_WriteChar(tecla);

                    __delay_ms(500);

                    hora_modificada[x] = tecla - '0';

                    /*
                     * Validación del formato HHMM
                     */
                    switch (x)
                    {
                        /* Primer dígito de la hora (0-2) x0:00 */
                        case 0:
                            if (hora_modificada[x] > 2)
                            {
                                Buzzer_WarningSound(&buzzer1);
                                LCD_I2C_SetCursor(3,14);
                                LCD_I2C_WriteString("x");
                                tecla = KEYPAD_NO_KEY;
                            }
                            break;

                        /* Segundo dígito de la hora (0-3) 0x:00 */
                        case 1:
                            if ((hora_modificada[0] == 2) && (hora_modificada[x] > 3))
                            {
                                Buzzer_WarningSound(&buzzer1);
                                LCD_I2C_SetCursor(3,15);
                                LCD_I2C_WriteString("x");
                                tecla = KEYPAD_NO_KEY;
                            }
                            break;

                        /* Decena de minutos (0-5)  00:x00 */
                        case 2:
                            if (hora_modificada[x] > 5)
                            {
                                Buzzer_WarningSound(&buzzer1);
                                LCD_I2C_SetCursor(3,17);
                                LCD_I2C_WriteString("x");
                                tecla = KEYPAD_NO_KEY;
                            }
                            break;
                    }
                    break;
            }

            /* Continuar con el siguiente dígito */
            if (tecla != KEYPAD_NO_KEY)
                break;
        }
    }
    
    /* Confirmación de guardado de hora */
    LCD_I2C_SetCursor(4,0);
    LCD_I2C_WriteString("[#]Guardar [*]Salir");
    
    while (1)
    {
        tecla = KEYPAD_NO_KEY;

        while (tecla == KEYPAD_NO_KEY)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);
        }

        switch (tecla)
        {
            case '#':

                Buzzer_ButtonClick(&buzzer1);
                __delay_ms(500);
                break;

            /* Cancelar operación */
            case '*':

                Buzzer_ButtonClick(&buzzer1);
                return;

            /* Cualquier otra tecla es inválida */
            default:

                Buzzer_WarningSound(&buzzer1);
                tecla = KEYPAD_NO_KEY;
        }

        if (tecla != KEYPAD_NO_KEY)
            break;
    }
    
    /*----------------------------------------------------------------------
     * Modificación del pastillero.
     *----------------------------------------------------------------------*/

    /*
        - EDITAR HORARIO 1 -
        Past. Actual:  P1 
        Nuevo Past. :  Px 
        Past:[1-4] Volver:*
    */
    
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1,0);
    LCD_I2C_WriteString("- EDITAR HORARIO _ -");
    LCD_I2C_SetCursor(1,17);
    LCD_I2C_WriteUInt8(horario_selecionado,1);
    
    LCD_I2C_SetCursor(2,0);
    LCD_I2C_WriteString("Past. Actual:  P_");
    LCD_I2C_SetCursor(2,16);
    dato_memoria = EEPROM_ReadByte(13+(5*(horario_selecionado-1))); // Pastillero actual
    LCD_I2C_WriteUInt8(dato_memoria,1);

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_WriteString("Nuevo Past. :  Px");
    
    LCD_I2C_SetCursor(4,0);
    LCD_I2C_WriteString("Past:[1-4] Volver:*");
    
    while (1)
    {
        tecla = KEYPAD_NO_KEY;

        while (tecla == KEYPAD_NO_KEY)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);
        }

        switch (tecla)
        {
            /* Pastilleros válidos */
            case '1':
            case '2':
            case '3':
            case '4':
                pastillero_modificado = tecla - '0';
                LCD_I2C_SetCursor(3,16);
                LCD_I2C_WriteChar(tecla);
                __delay_ms(500);
                break; // <-- BREAK AÑADIDO AQUI
                
            case '*':

                Buzzer_ButtonClick(&buzzer1);
                return;

            /* Cualquier otra tecla es inválida */
            default:

                Buzzer_WarningSound(&buzzer1);
                tecla = KEYPAD_NO_KEY;
        }

        /* Salir únicamente cuando la selección sea válida */
        if (tecla != KEYPAD_NO_KEY)
            break;
    }
    
    /* Confirmación de guardado de pastillero */
    LCD_I2C_SetCursor(4,0);
    LCD_I2C_WriteString("[#]Guardar [*]Salir");
    
    while (1)
    {
        tecla = KEYPAD_NO_KEY;

        while (tecla == KEYPAD_NO_KEY)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);
        }

        switch (tecla)
        {
            case '#':

                Buzzer_ButtonClick(&buzzer1);
                __delay_ms(500);
                break;

            /* Cancelar operación */
            case '*':

                Buzzer_ButtonClick(&buzzer1);
                return;

            /* Cualquier otra tecla es inválida */
            default:

                Buzzer_WarningSound(&buzzer1);
                tecla = KEYPAD_NO_KEY;
        }

        if (tecla != KEYPAD_NO_KEY)
            break;
    }

    /*----------------------------------------------------------------------
     * Guardar datos actualizados en EEPROM.
     *----------------------------------------------------------------------*/
    
    EEPROM_UpdateByte(9+(5*(horario_selecionado-1)), (hora_modificada[0]*10)+hora_modificada[1]);
    EEPROM_UpdateByte(10+(5*(horario_selecionado-1)), (hora_modificada[2]*10)+hora_modificada[3]);
    EEPROM_UpdateByte(13+(5*(horario_selecionado-1)), pastillero_modificado);
    EEPROM_UpdateByte(11+(5*(horario_selecionado-1)), 0);
    
    /*----------------------------------------------------------------------
     * Mensaje de confirmación final.
     *----------------------------------------------------------------------*/

    /*
       --------------------
        HORARIO ACTUALIZADO
         P1 a las 12:00   
       -------------------- 
    */
    
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1,0);
    LCD_I2C_WriteString("--------------------");

    LCD_I2C_SetCursor(2,1);
    LCD_I2C_WriteString("HORARIO ACTUALIZADO");

    LCD_I2C_SetCursor(3,2);
    LCD_I2C_WriteString("P_ a las __:__");
    
    LCD_I2C_SetCursor(3,3);
    dato_memoria = EEPROM_ReadByte(13 + ((horario_selecionado-1) * 5));
    LCD_I2C_WriteUInt8(dato_memoria,1);
    
    LCD_I2C_SetCursor(3,11);
    dato_memoria = EEPROM_ReadByte(9 + ((horario_selecionado-1) * 5));
    LCD_I2C_WriteUInt8(dato_memoria,2);
    
    LCD_I2C_SetCursor(3,14);
    dato_memoria = EEPROM_ReadByte(10 + ((horario_selecionado-1) * 5));
    LCD_I2C_WriteUInt8(dato_memoria,2);

    LCD_I2C_SetCursor(4,0);
    LCD_I2C_WriteString("--------------------");

    __delay_ms(2000);
}
/*
 * ============================================================================
 * FUNCIÓN: SubProceso_AgregarHorario
 * ============================================================================
 * Permite registrar un nuevo horario de dispensación de medicamentos.
 *
 * Flujo:
 *  1. Verifica que aún exista espacio para almacenar horarios.
 *  2. Solicita el número de pastillero (1-4).
 *  3. Solicita la hora en formato 24 horas (HHMM).
 *  4. Espera la confirmación del usuario.
 *  5. Guarda el horario en la EEPROM.
 *  6. Muestra un mensaje de confirmación.
 * ============================================================================
 */
void SubProceso_AgregarHorario(void)
{
    /*
     * Variables locales
     */
    uint8_t pastillero_selecionado;
    uint8_t index_horarios_ocupados;
    uint8_t hora[4] = {};

    /*----------------------------------------------------------------------
     * Verificar si existe espacio disponible para nuevos horarios.
     *----------------------------------------------------------------------*/
    if (EEPROM_ReadByte(1) >= 6)
    {
        LCD_I2C_Clear();
        LCD_I2C_SetCursor(2,3);
        LCD_I2C_WriteString("Horarios llenos");
        return;
    }

    /* Obtener la cantidad de horarios actualmente almacenados */
    index_horarios_ocupados = EEPROM_ReadByte(1);

    /*----------------------------------------------------------------------
     * Selección del pastillero.
     *----------------------------------------------------------------------*/

    /*
        -- NUEVO HORARIO ---
        Asignar pastillero:
        Numero (1-4): [ ]
        [*] Salir
    */

    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1,0);
    LCD_I2C_WriteString("-- NUEVO HORARIO ---");

    LCD_I2C_SetCursor(2,0);
    LCD_I2C_WriteString("Asignar pastillero:");

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_WriteString("Numero (1-4): [ ]");

    LCD_I2C_SetCursor(4,11);
    LCD_I2C_WriteString("[*] Salir");

    while (1)
    {
        tecla = KEYPAD_NO_KEY;

        while (tecla == KEYPAD_NO_KEY)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);
        }

        switch (tecla)
        {
            /* Pastilleros válidos */
            case '1':
            case '2':
            case '3':
            case '4':

                Buzzer_ButtonClick(&buzzer1);

                pastillero_selecionado = tecla - '0';

                LCD_I2C_SetCursor(3,15);
                LCD_I2C_WriteChar(tecla);

                __delay_ms(500);
                break;

            /* Cancelar operación */
            case '*':

                Buzzer_ButtonClick(&buzzer1);
                return;

            /* Cualquier otra tecla es inválida */
            default:

                Buzzer_WarningSound(&buzzer1);
                tecla = KEYPAD_NO_KEY;
        }

        /* Salir únicamente cuando la selección sea válida */
        if (tecla != KEYPAD_NO_KEY)
            break;
    }

    /*----------------------------------------------------------------------
     * Ingreso de la hora.
     *----------------------------------------------------------------------*/

    /*
        -- HORARIO PAST. X --
        Ingrese hora (24H):

            [ xx:xx ]

        [*] Salir
    */

    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1,0);
    LCD_I2C_WriteString("-- HORARIO PAST. _--");

    LCD_I2C_SetCursor(1,17);
    LCD_I2C_WriteUInt8(pastillero_selecionado,1);

    LCD_I2C_SetCursor(2,0);
    LCD_I2C_WriteString("Ingrese hora (24H):");

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_WriteString("     [ xx:xx ]");

    LCD_I2C_SetCursor(4,11);
    LCD_I2C_WriteString("[*] Salir");

    /*
     * Solicitar los cuatro dígitos:
     * HHMM
     */
    for (uint8_t x = 0; x < 4; x++)
    {
        while (1)
        {
            tecla = KEYPAD_NO_KEY;

            while (tecla == KEYPAD_NO_KEY)
            {
                tecla = Keypad_Read(&teclado);
                __delay_ms(20);
            }

            switch (tecla)
            {
                /* Teclas no permitidas */
                case 'A':
                case 'B':
                case 'C':
                case 'D':
                case '#':

                    Buzzer_WarningSound(&buzzer1);
                    tecla = KEYPAD_NO_KEY;
                    break;

                /* Cancelar ingreso */
                case '*':

                    Buzzer_ButtonClick(&buzzer1);
                    return;

                /* Se recibió un número */
                default:

                    Buzzer_ButtonClick(&buzzer1);

                    /* Mostrar el dígito en pantalla */
                    if (x <= 1)
                        LCD_I2C_SetCursor(3, x + 7);
                    else
                        LCD_I2C_SetCursor(3, x + 8);

                    LCD_I2C_WriteChar(tecla);

                    __delay_ms(500);

                    hora[x] = tecla - '0';

                    /*
                     * Validación del formato HHMM
                     */

                    switch (x)
                    {
                        /* Primer dígito de la hora (0-2) x0:00*/
                        case 0:

                            if (hora[x] > 2)
                            {
                                Buzzer_WarningSound(&buzzer1);

                                LCD_I2C_SetCursor(3,7);
                                LCD_I2C_WriteString("x");

                                tecla = KEYPAD_NO_KEY;
                            }
                            break;

                        /* Segundo dígito de la hora (0-3) 0x:00 */
                        case 1:

                            if ((hora[0] == 2) && (hora[x] > 3))
                            {
                                Buzzer_WarningSound(&buzzer1);

                                LCD_I2C_SetCursor(3,8);
                                LCD_I2C_WriteString("x");

                                tecla = KEYPAD_NO_KEY;
                            }
                            break;

                        /* Decena de minutos (0-5)  00:x00*/
                        case 2:

                            if (hora[x] > 5)
                            {
                                Buzzer_WarningSound(&buzzer1);

                                LCD_I2C_SetCursor(3,10);
                                LCD_I2C_WriteString("x");

                                tecla = KEYPAD_NO_KEY;
                            }
                            break;
                    }

                    break;
            }

            /* Continuar con el siguiente dígito */
            if (tecla != KEYPAD_NO_KEY)
                break;
        }
    }

    /*----------------------------------------------------------------------
     * Confirmación de guardado.
     *----------------------------------------------------------------------*/

    LCD_I2C_SetCursor(4,0);
    LCD_I2C_WriteString("[#]Guardar: [*]Salir");

    while (1)
    {
        tecla = KEYPAD_NO_KEY;

        while (tecla == KEYPAD_NO_KEY)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);
        }

        switch (tecla)
        {
            case '#':

                Buzzer_ButtonClick(&buzzer1);
                break;

            case '*':

                Buzzer_ButtonClick(&buzzer1);
                return;

            default:

                Buzzer_WarningSound(&buzzer1);
                tecla = KEYPAD_NO_KEY;
        }

        if (tecla != KEYPAD_NO_KEY)
            break;
    }

    /*----------------------------------------------------------------------
     * Guardar el nuevo horario en EEPROM.
     *----------------------------------------------------------------------*/

    EEPROM_UpdateByte(9 + (index_horarios_ocupados * 5),(hora[0] * 10) + hora[1]);

    EEPROM_UpdateByte(10 + (index_horarios_ocupados * 5),(hora[2] * 10) + hora[3]);

    EEPROM_UpdateByte(13 + (index_horarios_ocupados * 5),pastillero_selecionado);

    /* Actualizar cantidad de horarios almacenados */
    EEPROM_UpdateByte(1, index_horarios_ocupados + 1);

    /*----------------------------------------------------------------------
     * Mensaje de confirmación.
     *----------------------------------------------------------------------*/

    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1,0);
    LCD_I2C_WriteString("--------------------");

    LCD_I2C_SetCursor(2,2);
    LCD_I2C_WriteString("HORARIO GUARDADO");

    LCD_I2C_SetCursor(3,3);
    LCD_I2C_WriteString("P");

    LCD_I2C_WriteUInt8(pastillero_selecionado,1);
    LCD_I2C_WriteString(" a las ");

    dato_memoria = EEPROM_ReadByte(9 + (index_horarios_ocupados * 5));
    LCD_I2C_WriteUInt8(dato_memoria,2);

    LCD_I2C_WriteString(":");

    dato_memoria = EEPROM_ReadByte(10 + (index_horarios_ocupados * 5));
    LCD_I2C_WriteUInt8(dato_memoria,2);

    LCD_I2C_SetCursor(4,0);
    LCD_I2C_WriteString("--------------------");

    __delay_ms(2000);
}
/******************************************************************************
 * Función: SubProceso_RegistrarPastillas
 *----------------------------------------------------------------------------
 * Permite registrar una recarga de pastillas en uno de los cuatro
 * compartimientos disponibles.
 *
 * Flujo:
 *   1. Lee el stock total almacenado en EEPROM.
 *   2. Verifica que los datos sean válidos.
 *   3. Muestra el stock actual de cada pastillero.
 *   4. Permite seleccionar el compartimiento (1-4).
 *   5. Solicita la cantidad de pastillas a agregar.
 *   6. Guarda la nueva información al confirmar con '#'.
 *
 * Controles:
 *   1-4 -> Seleccionar pastillero.
 *   0-9 -> Cantidad a agregar.
 *   #   -> Guardar cambios.
 *   *   -> Cancelar operación.
 ******************************************************************************/
void SubProceso_RegistrarPastillas(void)
{
    uint8_t pastillero_selecionado;
    uint8_t cantidad_a_sumar = 0;

    /*==============================================================
     * Leer stock total de pastillas
     *==============================================================*/
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1,1);
    LCD_I2C_WriteString("REG. CANT. PASTILLAS");

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_WriteString("Leyendo Memoria");

    dato_memoria = EEPROM_ReadByte(2);

    MostrarAnimacionCarga(4,0);

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_ClearFile();

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_WriteString("Extrayendo Datos");

    MostrarAnimacionCarga(4,4);

    /*==============================================================
     * Validar datos leídos
     *==============================================================*/
    if (dato_memoria == 0xFF)
    {
        MostrarAnimacionCarga(4,8);

        LCD_I2C_SetCursor(3,0);
        LCD_I2C_ClearFile();

        LCD_I2C_SetCursor(3,2);
        LCD_I2C_WriteString("Datos Corruptos");

        __delay_ms(1500);
        return;
    }

    /*==============================================================
     * Mostrar stock total disponible
     *==============================================================*/
    LCD_I2C_SetCursor(3,0);
    LCD_I2C_ClearFile();

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_WriteString("Pastillas disp: ");
    LCD_I2C_WriteUInt8(dato_memoria,2);

    MostrarAnimacionCarga(4,8);

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_ClearFile();

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_WriteString("Preparando sistema..");

    MostrarAnimacionCarga(4,12);

    /*==============================================================
     * Mostrar stock individual de cada pastillero
     *==============================================================*/
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1,0);
    LCD_I2C_WriteString(" STOCK PASTILLEROS ");

    LCD_I2C_SetCursor(2,0);
    LCD_I2C_WriteString("P1:[");
    dato_memoria = EEPROM_ReadByte(5);
    LCD_I2C_WriteUInt8(dato_memoria,2);
    LCD_I2C_WriteString("]      P2:[");

    dato_memoria = EEPROM_ReadByte(6);
    LCD_I2C_WriteUInt8(dato_memoria,2);
    LCD_I2C_WriteString("]");

    LCD_I2C_SetCursor(3,0);
    LCD_I2C_WriteString("P3:[");

    dato_memoria = EEPROM_ReadByte(7);
    LCD_I2C_WriteUInt8(dato_memoria,2);
    LCD_I2C_WriteString("]      P4:[");

    dato_memoria = EEPROM_ReadByte(8);
    LCD_I2C_WriteUInt8(dato_memoria,2);
    LCD_I2C_WriteString("]");

    LCD_I2C_SetCursor(4,0);
    LCD_I2C_WriteString("Seleccione (1-4):[ ]");

    /*==============================================================
     * Esperar la selección del pastillero
     *==============================================================*/
    while (1)
    {
        tecla = KEYPAD_NO_KEY;

        while (tecla == KEYPAD_NO_KEY)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);
        }

        switch (tecla)
        {
            case '1':
            case '2':
            case '3':
            case '4':

                Buzzer_ButtonClick(&buzzer1);

                pastillero_selecionado = tecla - '0';

                LCD_I2C_SetCursor(4,18);
                LCD_I2C_WriteChar(tecla);

                __delay_ms(50);

                Detallar_CantPastillas(pastillero_selecionado);

                break;

            default:

                Buzzer_WarningSound(&buzzer1);

                LCD_I2C_SetCursor(4,18);
                LCD_I2C_WriteString("-");

                tecla = KEYPAD_NO_KEY;
        }

        /* Salir únicamente cuando la selección sea válida */
        if (tecla != KEYPAD_NO_KEY)
            break;
    }

    /*==============================================================
     * Esperar la cantidad de pastillas a registrar
     *==============================================================*/
    while (1)
    {
        tecla = KEYPAD_NO_KEY;

        while (tecla == KEYPAD_NO_KEY)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);
        }

        switch (tecla)
        {
            /*------------------------------------------
             * Teclas sin función en esta pantalla
             *-----------------------------------------*/
            case 'A':
            case 'B':
            case 'C':
            case 'D':

                Buzzer_WarningSound(&buzzer1);
                break;

            /*------------------------------------------
             * Cancelar operación
             *-----------------------------------------*/
            case '*':

                Buzzer_ButtonClick(&buzzer1);
                return;

            /*------------------------------------------
             * Confirmar y guardar datos
             *-----------------------------------------*/
            case '#':

                Buzzer_CorrectSound(&buzzer1);

                Guardar_CantPastillas(pastillero_selecionado,cantidad_a_sumar);
                __delay_ms(200);

                return;

            /*------------------------------------------
             * Registrar cantidad ingresada
             *-----------------------------------------*/
            default:

                Buzzer_ButtonClick(&buzzer1);

                cantidad_a_sumar = tecla - '0';

                LCD_I2C_SetCursor(3,18);
                LCD_I2C_WriteChar(tecla);

                break;
        }
    }
}
void Guardar_CantPastillas(uint8_t  pastillero_selecionado , uint8_t cantidad_a_sumar)
{
    const uint8_t ubicacion_memoria[] = {5, 6, 7, 8};
    dato_memoria = EEPROM_ReadByte(ubicacion_memoria[pastillero_selecionado-1]);//Leemos el dato de la memoria de -> comportimentx cantidad de pastillas
    EEPROM_UpdateByte(ubicacion_memoria[pastillero_selecionado-1],dato_memoria+cantidad_a_sumar);// Al pastillero indiviadual
    dato_memoria = EEPROM_ReadByte(2);//Leemos el valor de la direccion de -> cantidad de apstillas total
    EEPROM_UpdateByte(2,dato_memoria+ cantidad_a_sumar);// Cantidad de pastillas generales
    LCD_I2C_Clear();
    LCD_I2C_SetCursor(1, 0);
    LCD_I2C_WriteString("-- RECARGA EXITOSA--");
    LCD_I2C_SetCursor(2, 0);
    LCD_I2C_WriteString("PASTILLERO ");
    LCD_I2C_WriteUInt8(pastillero_selecionado,1);
    LCD_I2C_WriteString("LISTO");
    LCD_I2C_SetCursor(2, 0);
    LCD_I2C_WriteString("Total actual: [");
    dato_memoria = EEPROM_ReadByte(ubicacion_memoria[pastillero_selecionado-1]);
    LCD_I2C_WriteUInt8(dato_memoria,2);
    LCD_I2C_WriteString("]");
}
void Detallar_CantPastillas(uint8_t  tecla)
{
    const uint8_t ubicacion_memoria[] = {5, 6, 7, 8};
    LCD_I2C_Clear();
    LCD_I2C_SetCursor(1, 0);
    LCD_I2C_WriteString("-- RECARGA PAST. ");
    LCD_I2C_WriteUInt16(tecla,2,0);
    LCD_I2C_WriteString("--");
    LCD_I2C_SetCursor(2, 0);
    LCD_I2C_WriteString("Stock actual   : ");
    dato_memoria = EEPROM_ReadByte(ubicacion_memoria[tecla-1]);
    LCD_I2C_WriteUInt8(dato_memoria,2);
    LCD_I2C_SetCursor(3, 0);
    LCD_I2C_WriteString("A sumar        : 00");
    LCD_I2C_SetCursor(4, 0);
    LCD_I2C_WriteString("Guardar:[#] Salir:*");
}
void Detallar_Horarios(uint8_t numero,uint16_t hora,uint16_t minuto,uint16_t tipo)
{
    //1. 08:00 P1  5.18:00P4
    LCD_I2C_WriteUInt8(numero,1); // 1.
    LCD_I2C_WriteString(". ");
    dato_memoria = EEPROM_ReadByte(hora);//Hora
    LCD_I2C_WriteUInt8(dato_memoria,2);
    LCD_I2C_WriteString(":");
    dato_memoria = EEPROM_ReadByte(minuto);//Minuto
    LCD_I2C_WriteUInt8(dato_memoria,2);
    LCD_I2C_WriteString(" P");
    dato_memoria = EEPROM_ReadByte(tipo);//Tipo de compartimento
    LCD_I2C_WriteUInt8(dato_memoria,1);
}
void DataEEPROM(uint8_t data_memoria[40])
{
    for(uint8_t x = 0; x < 40; x++)
    {
        data_memoria[x] = EEPROM_ReadByte(x);
    }
}