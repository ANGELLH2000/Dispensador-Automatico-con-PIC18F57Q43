#include <xc.h>
#include <stdint.h>

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
static void SubProceso_ResetSistema();
static void SubProceso_MenuLCD();
static void SubProceso_VerHorarios();
static void mostrar_valor_cny70(uint8_t numero_sensor, uint16_t valor);
static void Detallar_Horarios(uint8_t numero,uint16_t hora,uint16_t minuto,uint16_t tipo);


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
    WS2812B_Init(&tira1, 2);
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
    config_motores();
    config_leds();
    config_buzzer();
    config_keypad();
    config_cny70();
    config_ir();
    config_i2c_lcd();
    //Configuracion de memoria
    EEPROM_WriteByte(1, 2);//Cantidad de Horario
    EEPROM_WriteByte(20, 8);//hora
    EEPROM_WriteByte(21, 9);//min
    EEPROM_WriteByte(22, 1);//tipo
    
    EEPROM_WriteByte(25, 9);//hora
    EEPROM_WriteByte(26, 21);//min
    EEPROM_WriteByte(27, 3);//tipo
}

/*==============================================================================
 * VISUALIZACIÓN DE LOS VALORES CNY70
 *============================================================================*/

static void mostrar_valor_cny70(uint8_t numero_sensor, uint16_t valor)
{
    /*
     * Limpia la pantalla y muestra el número del sensor.
     */
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1, 0);
    LCD_I2C_WriteString("Sensor ");
    LCD_I2C_WriteInt(numero_sensor);

    /*
     * Muestra el valor obtenido mediante el ADC.
     */
    LCD_I2C_SetCursor(2, 0);
    LCD_I2C_WriteString("Valor: ");
    LCD_I2C_WriteInt(valor);

    /*
     * Mantiene el valor visible durante un segundo.
     */
    __delay_ms(1000);
}

/*==============================================================================
 * LECTURA DE LOS SENSORES CNY70
 *============================================================================*/

void lectura_cny70(void)
{
    
    /*
     * Lee el primer sensor y muestra su valor.
     */
    valor_S1 = CNY70_Read(&sensor1);
    mostrar_valor_cny70(1, valor_S1);

    /*
     * Lee el segundo sensor y muestra su valor.
     */
    valor_S2 = CNY70_Read(&sensor2);
    mostrar_valor_cny70(2, valor_S2);

    /*
     * Lee el tercer sensor y muestra su valor.
     */
    valor_S3 = CNY70_Read(&sensor3);
    mostrar_valor_cny70(3, valor_S3);

    /*
     * Lee el cuarto sensor y muestra su valor.
     */
    valor_S4 = CNY70_Read(&sensor4);
    mostrar_valor_cny70(4, valor_S4);
}

/*==============================================================================
 * VERIFICACIÓN DE LAS CONDICIONES INICIALES
 *============================================================================*/

void verificar_condiciones_iniciales(void)
{
    /*
     * Verifica que la dirección del LCD se haya configurado
     * correctamente.
     */
    if (estado != I2C_OK)
    {
        sistem_error("I2C sin conexion");
    }

    /*
     * Lee la fecha y la hora almacenadas en el DS1307.
     */
    estado = DS1307_ReadDateTime(&fechaHora);

    /*
     * Comprueba si el RTC respondió mediante el bus I2C.
     */
    if (estado != I2C_OK)
    {
        sistem_error("RTC sin conexion");
    }

    /*
     * clock_running igual a cero indica que el bit CH
     * está en uno y el oscilador se encuentra detenido.
     */
    if (fechaHora.clock_running == 0)
    {
        sistem_error("RTC detenido");
    }

    /*
     * data_valid igual a cero indica que la fecha o la hora
     * almacenada contiene valores fuera del rango permitido.
     */
    if (fechaHora.data_valid == 0)
    {
        sistem_error("Fecha invalida");
    }

    /*
     * Falta implementar la comprobación de los datos
     * almacenados en la memoria EEPROM o DFM.
     */
    /* TODO: verificar la memoria EEPROM/DFM. */

    /*
     * Indica que todas las comprobaciones iniciales
     * finalizaron correctamente.
     */
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1, 0);
    LCD_I2C_WriteString("Sistema correcto");

    /*
     * Enciende la tira LED en color verde.
     */
    WS2812B_RGB(&tira1, 0, 200, 0);

    __delay_ms(1000);
    //SubProceso_ResetSistema();
    SubProceso_MenuLCD();
    __delay_ms(2000);        
    while(1){
        lectura_cny70();
        __delay_ms(1000);
    }
    
}

/*==============================================================================
 * SISTEMA DE ERROR
 *============================================================================*/

void sistem_error(const char *mensaje)
{
    /*
     * Limpia la pantalla y muestra el mensaje de error.
     */
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1, 0);
    LCD_I2C_WriteString("Error en sistema");

    LCD_I2C_SetCursor(2, 0);
    LCD_I2C_WriteString(mensaje);

    /*
     * Enciende la tira LED en color rojo.
     */
    WS2812B_RGB(&tira1, 200, 0, 0);

    /*
     * Detiene el programa para evitar que el sistema
     * continúe funcionando después del error.
     */
    while (1)
    {
        __delay_ms(1000);
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
 * Función: SubProceso_ResetSistema
 * ---------------------------------------------------------------------------
 * Reinicia el sistema mostrando el avance del proceso en la pantalla LCD.
 *
 * Proceso:
 *      1. Mostrar mensaje de reinicio.
 *      2. Apagar buzzer.
 *      3. Apagar LEDs.
 *      4. Limpiar variables.
 *      5. Finalizar el reinicio.
 ******************************************************************************/
void SubProceso_ResetSistema(void)
{
    /*==============================================================
        Pantalla de inicio
    ==============================================================*/
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1, 4);
    LCD_I2C_WriteString("Reiniciando");

    LCD_I2C_SetCursor(2, 6);
    LCD_I2C_WriteString("Sistema");
    __delay_ms(1000);

    /*==============================================================
        Paso 1: Apagar buzzer
    ==============================================================*/
    LCD_I2C_SetCursor(3, 4);
    LCD_I2C_WriteString("Apagando Buzzer");

    Buzzer_Off(&buzzer1);

    MostrarAnimacionCarga(4, 2);

    LCD_I2C_SetCursor(3, 0);
    LCD_I2C_ClearFile();


    /*==============================================================
        Paso 2: Apagar LEDs
    ==============================================================*/
    LCD_I2C_SetCursor(3, 4);
    LCD_I2C_WriteString("Apagando LEDs");

    WS2812B_Clear(&tira1);

    MostrarAnimacionCarga(4, 6);

    LCD_I2C_SetCursor(3, 0);
    LCD_I2C_ClearFile();


    /*==============================================================
        Paso 3: Limpiar variables
    ==============================================================*/
    LCD_I2C_SetCursor(3, 1);
    LCD_I2C_WriteString("Limpiando Variables");

    /**************************************************************
     * Reiniciar aquí todas las variables del sistema
     **************************************************************/
    // contador = 0;
    // estado = 0;
    // alarma = false;
    // etc...

    MostrarAnimacionCarga(4, 10);

    LCD_I2C_SetCursor(3, 0);
    LCD_I2C_ClearFile();


    /*==============================================================
        Paso 4: Finalizar reinicio
    ==============================================================*/
    LCD_I2C_SetCursor(3, 5);
    LCD_I2C_WriteString("Reseteando");

    MostrarAnimacionCarga(4, 14);

    /*==============================================================
        Reinicio final
    ==============================================================*/
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(2, 5);
    LCD_I2C_WriteString("Sistema");

    LCD_I2C_SetCursor(3, 4);
    LCD_I2C_WriteString("Reiniciado");

    __delay_ms(1000);
    LCD_I2C_Clear();
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

        LCD_I2C_SetCursor(1, 5);
        LCD_I2C_WriteString("Seleccionar");

        LCD_I2C_SetCursor(2, 3);
        LCD_I2C_WriteString("Opcion Deseada");

        __delay_ms(2000);


        /*==============================================================
         * Mostrar menú principal
         *==============================================================*/
        LCD_I2C_Clear();

        LCD_I2C_SetCursor(1, 0);
        LCD_I2C_WriteString("1.Agr.Hor  2.Mod.Hor");

        LCD_I2C_SetCursor(2, 0);
        LCD_I2C_WriteString("3.Ver horarios");

        LCD_I2C_SetCursor(3, 0);
        LCD_I2C_WriteString("4.Reg. pastillas");

        LCD_I2C_SetCursor(4, 0);
        LCD_I2C_WriteString("*.Reset sistema");


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
                        LCD_I2C_SetCursor(1, 7);
                        LCD_I2C_WriteString("Opcion");
                        LCD_I2C_SetCursor(2, 2);
                        LCD_I2C_WriteString("Agregar Horario");
                        __delay_ms(2000);
                        return;

                    case '2':
                        LCD_I2C_SetCursor(1, 7);
                        LCD_I2C_WriteString("Opcion");
                        LCD_I2C_SetCursor(2, 1);
                        LCD_I2C_WriteString("Modificar Horario");
                        __delay_ms(2000);
                        return;

                    case '3':
                        LCD_I2C_SetCursor(1, 7);
                        LCD_I2C_WriteString("Opcion");
                        LCD_I2C_SetCursor(2, 4);
                        LCD_I2C_WriteString("Ver Horarios");
                        __delay_ms(2000);
                        SubProceso_VerHorarios();
                        break;

                    case '4':
                        LCD_I2C_SetCursor(1, 7);
                        LCD_I2C_WriteString("Opcion");
                        LCD_I2C_SetCursor(2, 1);
                        LCD_I2C_WriteString("Registrar Pastilla");
                        __delay_ms(2000);
                        return;

                    case '*':
                        LCD_I2C_SetCursor(1, 7);
                        LCD_I2C_WriteString("Opcion");
                        LCD_I2C_SetCursor(2, 3);
                        LCD_I2C_WriteString("Reset sistema");
                        __delay_ms(2000);
                        SubProceso_ResetSistema();
                        return;

                    default:
                        /* Opción inválida: informar al usuario y
                           volver a mostrar el menú. */
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
void SubProceso_VerHorarios(void)
{
    /*==============================================================
     * Leer cantidad de horarios almacenados
     *==============================================================*/
    LCD_I2C_Clear();

    LCD_I2C_SetCursor(1, 6);
    LCD_I2C_WriteString("Horarios");

    LCD_I2C_SetCursor(3, 0);
    LCD_I2C_WriteString("Leyendo Memoria");

    dato_memoria = EEPROM_ReadByte(1);

    MostrarAnimacionCarga(4, 0);

    LCD_I2C_SetCursor(3, 0);
    LCD_I2C_ClearFile();

    LCD_I2C_SetCursor(3, 2);
    LCD_I2C_WriteString("Extrayendo Datos");

    MostrarAnimacionCarga(4, 4);

    /*==============================================================
     * Validar datos
     *==============================================================*/
    if (dato_memoria == 0xFF || dato_memoria > 6)
    {
        MostrarAnimacionCarga(4, 8);

        LCD_I2C_SetCursor(3, 0);
        LCD_I2C_ClearFile();

        LCD_I2C_SetCursor(3, 2);
        LCD_I2C_WriteString("Datos Corruptos");

        __delay_ms(1500);
        return;
    }

    if (dato_memoria == 0)
    {
        MostrarAnimacionCarga(4, 8);

        LCD_I2C_SetCursor(3, 0);
        LCD_I2C_ClearFile();

        LCD_I2C_SetCursor(3, 0);
        LCD_I2C_WriteString("Sin Hor. Registrados");

        __delay_ms(1500);
        return;
    }

    /*==============================================================
     * Preparar datos
     *==============================================================*/
    LCD_I2C_SetCursor(3, 0);
    LCD_I2C_ClearFile();

    LCD_I2C_SetCursor(3, 2);
    LCD_I2C_WriteString("Preparando Datos");

    MostrarAnimacionCarga(4, 8);
    MostrarAnimacionCarga(4, 12);

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
        LCD_I2C_Clear();

        LCD_I2C_SetCursor(1, 0);
        LCD_I2C_WriteString("--- MIS HORARIOS ---");

        switch (pagina)
        {
            case 0:

                if (cant_horarios >= 1)
                {
                    LCD_I2C_SetCursor(2, 0);
                    Detallar_Horarios(1,20,21,22);
                }

                if (cant_horarios >= 2)
                {
                    LCD_I2C_SetCursor(3, 0);
                    Detallar_Horarios(2,25,26,27);
                }

                if (cant_horarios >= 3)
                {
                    LCD_I2C_SetCursor(4, 0);
                    Detallar_Horarios(3,29,30,31);
                }

                break;

            case 1:

                if (cant_horarios >= 4)
                {
                    LCD_I2C_SetCursor(2, 0);
                    Detallar_Horarios(4,33,34,35);
                }

                if (cant_horarios >= 5)
                {
                    LCD_I2C_SetCursor(3, 0);
                    Detallar_Horarios(5,37,38,39);
                }

                if (cant_horarios >= 6)
                {
                    LCD_I2C_SetCursor(4, 0);
                    Detallar_Horarios(6,41,42,43);
                }

                break;
        }

        /*==========================================================
         * Mostrar ayudas de navegación
         *==========================================================*/
        if (pagina > 0)
        {   
            LCD_I2C_SetCursor(2, 12);
            LCD_I2C_WriteString("[A]Subir");
        }

        if (pagina < paginas_max)
        {
            LCD_I2C_SetCursor(4, 12);
            LCD_I2C_WriteString("[B]Bajar");
        }
        LCD_I2C_SetCursor(3, 12);
        LCD_I2C_WriteString("[*]Salir");
        /*==========================================================
         * Esperar tecla
         *==========================================================*/
        tecla = KEYPAD_NO_KEY;

        while (tecla == KEYPAD_NO_KEY)
        {
            tecla = Keypad_Read(&teclado);
            __delay_ms(20);
        }

        /* Esperar que el usuario suelte la tecla */
        while (Keypad_Read(&teclado) != KEYPAD_NO_KEY)
        {
            __delay_ms(20);
        }

        /*==========================================================
         * Procesar tecla
         *==========================================================*/
        switch (tecla)
        {
            case 'A':

                if (pagina > 0)
                    pagina--;

                break;

            case 'B':

                if (pagina < paginas_max)
                    pagina++;

                break;

            case '*':
                return;
        }
    }
}
void Detallar_Horarios(uint8_t numero,uint16_t hora,uint16_t minuto,uint16_t tipo)
{
    //1. 08:00 P1  5.18:00P4
    LCD_I2C_WriteUInt8(numero,1);
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