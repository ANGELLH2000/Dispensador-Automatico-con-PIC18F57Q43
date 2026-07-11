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
            time.sleep(0.1) 
            
            
            
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
    
    def _read_loop(self):
        """Bucle asíncrono que implementa la máquina de estados de validación."""
        while self.running:
            if self.ser and self.ser.is_open:
                try:
                    # 1. Buscar la cabecera (AA 55) bloqueante en el flujo entrante
                    if self.ser.read(1) == b'\xAA':
                        if self.ser.read(1) == b'\x55':
                            
                            # 2. Leer la longitud (1 byte)
                            longitud_byte = self.ser.read(1)
                            if not longitud_byte: continue
                            longitud = longitud_byte[0]
                            
                            if longitud == 40:
                                # 3. Leer los 40 bytes de Payload (Datos)
                                payload = self.ser.read(40)
                                if len(payload) < 40: continue
                                
                                # 4. Leer el Checksum enviado por el PIC (1 byte)
                                checksum_pic_byte = self.ser.read(1)
                                if not checksum_pic_byte: continue
                                checksum_pic = checksum_pic_byte[0]
                                
                                # Leer el salto de línea residual (0x0A) para limpiar el buffer
                                self.ser.read(1)
                                
                                # 5. Tu validación matemática de integridad
                                checksum_calculado = sum(payload) % 256
                                
                                if checksum_calculado == checksum_pic:
                                    # Desempaquetar los 40 bytes en una tupla de enteros (0-255)
                                    datos_puros = struct.unpack('<40B', payload)
                                    
                                    # Enviamos la tupla limpia a la interfaz gráfica
                                    if self.data_callback:
                                        self.data_callback(datos_puros)
                                else:
                                    print(f"[ERROR CHECKSUM]: PIC: {hex(checksum_pic)} | PC: {hex(checksum_calculado)}")
                            else:
                                print(f"[ERROR LONGITUD]: Se esperaban 40, llegó {longitud}")
                                
                except Exception as e:
                    print(f"[ERROR LECTURA]: Canal interrumpido. {e}")
                    self.disconnect()
                    break
            time.sleep(0.002) # Mantiene la CPU descansada