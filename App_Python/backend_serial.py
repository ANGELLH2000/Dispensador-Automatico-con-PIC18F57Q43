"""
MÓDULO DE COMUNICACIÓN UART - PARSER BINARIO VALIDADO
Encargado de la conexión, búsqueda de cabecera, validación por Checksum,
desempaquetado de datos y transmisión de comandos hacia el PIC18F57Q43.
"""
import serial
import serial.tools.list_ports
import threading
import time
import struct 

class PICSerialBackend:
    def __init__(self, data_callback=None):
        self.ser = None
        self.is_connected = False
        self.read_thread = None
        self.running = False
        self.data_callback = data_callback 

    @staticmethod
    def list_available_ports():
        """Devuelve únicamente los puertos COM reales disponibles en el sistema."""
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]

    def connect(self, port, baudrate=9600):
        """Abre el puerto serial, inicia el hilo de escucha y envía la petición inicial."""
        if self.is_connected:
            return True
        try:
            self.ser = serial.Serial(port, int(baudrate), timeout=1)
            self.is_connected = True
            self.running = True
            
            # 1. Iniciamos el hilo para estar listos para escuchar la respuesta
            self.read_thread = threading.Thread(target=self._read_loop, daemon=True)
            self.read_thread.start()
            
            # 2. Pequeña pausa para estabilizar la apertura del puerto COM
            time.sleep(1) 
            
            
            
            return True
        except Exception as e:
            print(f"[ERROR CONEXIÓN]: {e}")
            self.is_connected = False
            return False

    def disconnect(self):
        """Cierra el puerto de forma segura."""
        self.running = False
        self.is_connected = False
        if self.ser and self.ser.is_open:
            self.ser.close()
        print("[SISTEMA]: Puerto serial cerrado.")

    # ========================================================================
    # FUNCIONES DE TRANSMISIÓN (PC -> PIC)
    # ========================================================================
    
    def _send_frame(self, command: int, data_bytes: list):
        """
        Construye y envía una trama completa hacia el PIC.
        Estructura: [0xAA, Comando, Longitud, Datos..., Checksum, 0x0A]
        """
        if not self.is_connected or not self.ser or not self.ser.is_open:
            print("[ERROR ENVÍO]: El puerto no está abierto.")
            return False
            
        try:
            frame = bytearray([0xAA, command])  # Cabecera 1 y Cabecera 2 (Comando)
            frame.append(len(data_bytes))       # Byte de longitud
            frame.extend(data_bytes)            # Payload de datos
            
            # Checksum: Suma de todos los datos enviados (solo payload) % 256
            checksum = sum(data_bytes) % 256
            frame.append(checksum)
            
            frame.append(0x0A)                  # Byte de Fin
            
            self.ser.write(frame)
            print(f"[PC -> PIC]: Tramitida: {frame.hex(' ').upper()}")
            return True
            
        except Exception as e:
            print(f"[ERROR ENVÍO TRAMA]: {e}")
            return False

    def request_memory_data(self):
        """Envía 0xAA 0x56. Longitud: 1 (Letra 'R' = 0x52)."""
        self._send_frame(0x56, [ord('R')])

    def write_schedule_data(self, horario: int, hora: int, minuto: int, compartimento: int):
        """Envía 0xAA 0x15. Longitud: 4 (Horario, Hora, Minuto, Compartimento)."""
        self._send_frame(0x15, [horario, hora, minuto, compartimento])

    def write_pillbox_data(self, cantidad: int, compartimento: int):
        """Envía 0xAA 0x07. Longitud: 2 (Cantidad, Compartimento)."""
        self._send_frame(0x07, [cantidad, compartimento])

    def test_system_functions(self, tipo: int):
        """Envía 0xAA 0x26. Longitud: 1 (Tipo de test)."""
        self._send_frame(0x26, [tipo])


    # ========================================================================
    # FUNCIÓN DE RECEPCIÓN (PIC -> PC)
    # ========================================================================
    
    def request_diagnostic_data(self):
        """Envía 0xAA 0x26 0x01 0x01 [CHK] 0x0A para pedir los sensores."""
        self._send_frame(0x26, [0x01])

    # ========================================================================
    # FUNCIÓN DE RECEPCIÓN (PIC -> PC)
    # ========================================================================
    
    def _read_loop(self):
        """Bucle asíncrono que implementa la máquina de estados de validación."""
        while self.running:
            if self.ser and self.ser.is_open:
                try:
                    if self.ser.read(1) == b'\xAA':
                        if self.ser.read(1) == b'\x55':
                            
                            # Leer longitud dinámicamente
                            longitud_byte = self.ser.read(1)
                            if not longitud_byte: continue
                            longitud = longitud_byte[0]
                            
                            # ¡NUEVO!: Leer exactamente la cantidad de bytes que diga 'longitud'
                            payload = self.ser.read(longitud)
                            if len(payload) < longitud: continue
                            
                            # Leer Checksum
                            checksum_pic_byte = self.ser.read(1)
                            if not checksum_pic_byte: continue
                            checksum_pic = checksum_pic_byte[0]
                            
                            # Leer 0x0A final
                            self.ser.read(1)
                            
                            # Validar Checksum
                            checksum_calculado = sum(payload) % 256
                            
                            if checksum_calculado == checksum_pic:
                                # Convertimos el payload a una tupla estándar
                                datos_puros = tuple(payload)
                                if self.data_callback:
                                    self.data_callback(datos_puros)
                            else:
                                print(f"[ERROR CHECKSUM]: PIC: {hex(checksum_pic)} | PC: {hex(checksum_calculado)}")
                                
                except Exception as e:
                    print(f"[ERROR LECTURA]: Canal interrumpido. {e}")
                    self.disconnect()
                    break
            time.sleep(0.5)