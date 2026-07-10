"""
MÓDULO DE COMUNICACIÓN UART - PARSER BINARIO VALIDADO
Encargado de la conexión, búsqueda de cabecera, validación por Checksum 
y desempaquetado de los 40 bytes de payload del PIC18F57Q43.
"""
import serial
import serial.tools.list_ports
import threading
import time
import struct # Necesario para struct.unpack

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
        """Abre el puerto serial para recibir la ráfaga binaria."""
        if self.is_connected:
            return True
        try:
            self.ser = serial.Serial(port, int(baudrate), timeout=1)
            self.is_connected = True
            self.running = True
            
            self.read_thread = threading.Thread(target=self._read_loop, daemon=True)
            self.read_thread.start()
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

    def _read_loop(self):
        """Bucle asíncrono que implementa tu máquina de estados de validación."""
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