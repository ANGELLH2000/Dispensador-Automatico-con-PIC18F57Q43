"""
================================================================================
  DISPENSADOR AUTOMATIZADO DE PASTILLAS INTELIGENTE
================================================================================
  Archivo          : main.py
  Microcontrolador : PIC18F57Q43
  Versión          : 2.0.2
  Descripción      : Prototipo visual completo con datos estáticos (dummy).
                     Sin lógica de backend ni comunicación serial real.
================================================================================
  FLUJO DE NAVEGACIÓN:
    1) SplashScreen    →  (sin sidebar)
    2) ConnectionPage  →  (sin sidebar)
    3) Clic "Conectar" →  Se instancia Sidebar + Dashboard
================================================================================
"""

# ------------------------------------------------------------------------------
# IMPORTS
# ------------------------------------------------------------------------------
import customtkinter as ctk
from datetime import datetime
import random

from backend_serial import PICSerialBackend

# ------------------------------------------------------------------------------
# CONSTANTES Y CONFIGURACIÓN GLOBAL
# ------------------------------------------------------------------------------
APP_NAME    = "Dispensador Automatizado de Pastillas"
APP_VERSION = "v2.0.2"
APP_BUILD_DATE = "2026-07-10"
NOMBRE_PASTILLA_COMPARTMENTOS = ["Paracetamol 500mg", "Ibuprofeno 200mg", "Enalapril 10mg", "Metformina 500mg"]

WINDOW_W = 1024
WINDOW_H = 880
CAPACITY_MAX = 12  # Capacidad máxima de pastillas por compartimento

CORNER = 12  # corner_radius global

# Paleta — modo oscuro industrial
C = {
    "bg"          : "#1A1A2E",
    "bg_alt"      : "#16213E",
    "sidebar"     : "#0F3460",
    "card"        : "#1C2541",
    "card_hover"  : "#243B6A",
    "border"      : "#2A3A5C",
    "input"       : "#141E33",
    "text"        : "#EAEAEA",
    "text2"       : "#A8B2D1",
    "muted"       : "#5C6B8A",
    "accent"      : "#4F8CFF",
    "accent_hover": "#3D7AEE",
    "purple"      : "#7B61FF",
    "green"       : "#22C55E",
    "red"         : "#EF4444",
    "yellow"      : "#FACC15",
    "orange"      : "#F97316",
    "divider"     : "#263354",
    "terminal"    : "#0D0D0D",
    "term_text"   : "#00FF41",
}

FONTS = {"title": 24, "sub": 18, "body": 14, "small": 12, "tiny": 10}

PORTS     = PICSerialBackend.list_available_ports()
if not PORTS:
            PORTS = ["No se detectaron puertos activos"]
BAUDRATES = ["9600"]


# ------------------------------------------------------------------------------
# HELPERS / COMPONENTES DE INTERFAZ
# ------------------------------------------------------------------------------

# ============================================================================
# FUNCIÓN: font
# ============================================================================
# Retorna CTkFont con la familia y tamaño del sistema de diseño.
#
# Flujo:
#  1. Busca el tamaño de fuente en el diccionario FONTS.
#  2. Asigna el peso (bold o normal).
#  3. Retorna el objeto CTkFont.
# ============================================================================
def font(size_key="body", bold=False):
    return ctk.CTkFont(family="Segoe UI", size=FONTS.get(size_key, 14),
                       weight="bold" if bold else "normal")


# ============================================================================
# FUNCIÓN: dim
# ============================================================================
# Mezcla *hex_color* con el fondo de card para simular transparencia.
#
# Flujo:
#  1. Extrae componentes RGB del color ingresado.
#  2. Combina los valores con el color de fondo usando un factor alpha.
#  3. Retorna el nuevo color en formato hexadecimal o el color por defecto.
# ============================================================================
def dim(hex_color: str, alpha: float = 0.15) -> str:
    # Variables locales
    # r, g, b : Componentes del color base
    # br, bg_, bb : Componentes del color de fondo (card)
    try:
        r, g, b = int(hex_color[1:3], 16), int(hex_color[3:5], 16), int(hex_color[5:7], 16)
        br, bg_, bb = 0x1C, 0x25, 0x41  # C["card"]
        return f"#{int(r*alpha+br*(1-alpha)):02X}" \
               f"{int(g*alpha+bg_*(1-alpha)):02X}" \
               f"{int(b*alpha+bb*(1-alpha)):02X}"
    except Exception:
        return C["card"]


# ============================================================================
# FUNCIÓN: card
# ============================================================================
# Factory: crea un CTkFrame con estilo visual de "tarjeta" o panel.
#
# Flujo:
#  1. Aplica un diccionario de estilos predeterminados.
#  2. Sobrescribe con argumentos adicionales.
#  3. Retorna la instancia de CTkFrame configurada.
# ============================================================================
def card(parent, **kw):
    defaults = dict(fg_color=C["card"], corner_radius=CORNER,
                    border_width=1, border_color=C["border"])
    defaults.update(kw)
    return ctk.CTkFrame(parent, **defaults)


# ============================================================================
# FUNCIÓN: accent_btn
# ============================================================================
# Factory: crea un botón con estilo de color de acento principal.
# ============================================================================
def accent_btn(parent, text, command=None, color=None, **kw):
    fg = color or C["accent"]
    defaults = dict(text=text, command=command, font=font("body", True),
                    fg_color=fg, hover_color=C["accent_hover"],
                    text_color=C["text"], corner_radius=CORNER,
                    height=42, cursor="hand2")
    defaults.update(kw)
    return ctk.CTkButton(parent, **defaults)


# ============================================================================
# FUNCIÓN: ghost_btn
# ============================================================================
# Factory: crea un botón estilo ghost/outline (borde sin fondo relleno).
# ============================================================================
def ghost_btn(parent, text, command=None, **kw):
    defaults = dict(text=text, command=command, font=font("small", True),
                    fg_color="transparent", hover_color=C["card_hover"],
                    text_color=C["text2"], border_width=1,
                    border_color=C["border"], corner_radius=CORNER,
                    height=36, cursor="hand2")
    defaults.update(kw)
    return ctk.CTkButton(parent, **defaults)


# ============================================================================
# FUNCIÓN: sep
# ============================================================================
# Crea un separador horizontal de 1px.
# ============================================================================
def sep(parent):
    return ctk.CTkFrame(parent, height=1, fg_color=C["divider"], corner_radius=0)


# ------------------------------------------------------------------------------
# CLASES DE INTERFAZ GRÁFICA
# ------------------------------------------------------------------------------

# ============================================================================
# CLASE / FUNCIÓN: SplashScreen
# ============================================================================
# Primera pantalla de la aplicación.
# Muestra título, descripción y botón "Continuar a Conexión".
# NO tiene menú lateral.
#
# Flujo:
#  1. Inicializa el frame ocupando toda la ventana.
#  2. Llama al constructor visual _build().
# ============================================================================
class SplashScreen(ctk.CTkFrame):

    def __init__(self, parent, on_continue):
        super().__init__(parent, fg_color=C["bg"], corner_radius=0)
        self.on_continue = on_continue
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self._build()

    # ========================================================================
    # FUNCIÓN: _build (SplashScreen)
    # ========================================================================
    # Construye los widgets internos de la pantalla de inicio.
    # ========================================================================
    def _build(self):
        # Variables locales / Contenedores
        center = ctk.CTkFrame(self, fg_color="transparent")
        center.grid(row=0, column=0)

        # ----------------------------------------------------------------------
        # Badge y Logo
        # ----------------------------------------------------------------------
        ctk.CTkLabel(center, text=f"  {APP_VERSION}  ", font=font("small", True),
                     text_color=C["purple"], fg_color=dim(C["purple"], 0.18),
                     corner_radius=8, height=26).pack(pady=(0, 24))

        logo_outer = ctk.CTkFrame(center, width=120, height=120,
                                  fg_color=C["purple"], corner_radius=30)
        logo_outer.pack(pady=(0, 32))
        logo_outer.pack_propagate(False)

        logo_inner = ctk.CTkFrame(logo_outer, width=88, height=88,
                                  fg_color="#1A1040", corner_radius=22)
        logo_inner.place(relx=0.5, rely=0.5, anchor="center")
        logo_inner.pack_propagate(False)

        ctk.CTkLabel(logo_inner, text="💊", font=ctk.CTkFont(size=42),
                     text_color=C["text"]).place(relx=0.5, rely=0.5, anchor="center")

        # ----------------------------------------------------------------------
        # Textos y Botón Principal
        # ----------------------------------------------------------------------
        ctk.CTkLabel(center, text="Dispensador Automatizado",
                     font=font("title", True), text_color=C["text"]).pack()
        ctk.CTkLabel(center, text="de Pastillas",
                     font=font("sub"), text_color=C["purple"]).pack(pady=(0, 4))

        ctk.CTkFrame(center, width=300, height=2,
                     fg_color=C["purple"], corner_radius=1).pack(pady=20)

        ctk.CTkLabel(
            center,
            text="Proyecto de Ingeniería Mecatrónica 2026 \n"
                 "Microcontrolador: PIC18F57Q43",
            font=font("small"), text_color=C["text2"], justify="center",
        ).pack(pady=(0, 40))

        accent_btn(center, text="   Comenzar a Conexión   →",
                   command=self.on_continue, width=280, height=50).pack()

        ctk.CTkLabel(center, text="Alumnos:\nArmas Pérez Alcides Gabriel (u202013642); \nOrtiz Fabian Emanuel Nazario (u202223704); \nLeón Honores Angel Gabriel (u201717039); \nVega Rodriguez Zeus Steve (u2021c589)",
                     font=font("small"), text_color=C["muted"]).pack(pady=(32, 0))
        ctk.CTkLabel(center, text="Profesores:\nIng. Lau Gan , Kalun José, Ing. Vértiz Linares ,Saul Nóe",
                     font=font("small"), text_color=C["muted"]).pack(pady=(32, 0))


# ============================================================================
# CLASE / FUNCIÓN: ConnectionPage
# ============================================================================
# Segunda pantalla: selección de puerto serial y baudrate.
#
# Flujo:
#  1. Muestra un panel interactivo para configurar la conexión.
#  2. Al presionar "Conectar", invoca la función para iniciar serial.
#  3. Si hay éxito, levanta el Workspace con Sidebar.
# ============================================================================
class ConnectionPage(ctk.CTkFrame):

    def __init__(self, parent, on_connect, on_back):
        super().__init__(parent, fg_color=C["bg"], corner_radius=0)
        self.on_connect = on_connect
        self.on_back    = on_back
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self._build()

    # ========================================================================
    # FUNCIÓN: _build (ConnectionPage)
    # ========================================================================
    def _build(self):
        # Panel central tipo card
        panel = card(self, width=460, border_width=2)
        panel.grid(row=0, column=0)
        panel.grid_columnconfigure(0, weight=1)

        # ----------------------------------------------------------------------
        # Encabezado del Panel
        # ----------------------------------------------------------------------
        hdr = ctk.CTkFrame(panel, fg_color="transparent")
        hdr.grid(row=0, column=0, sticky="ew", padx=32, pady=(32, 8))

        icon_bg = ctk.CTkFrame(hdr, width=48, height=48,
                               fg_color=dim(C["accent"], 0.20), corner_radius=12)
        icon_bg.pack(side="left")
        icon_bg.pack_propagate(False)
        ctk.CTkLabel(icon_bg, text="🔌", font=ctk.CTkFont(size=24)).place(
            relx=0.5, rely=0.5, anchor="center")

        title_box = ctk.CTkFrame(hdr, fg_color="transparent")
        title_box.pack(side="left", padx=(12, 0))
        ctk.CTkLabel(title_box, text="Conexión al Sistema",
                     font=font("sub", True), text_color=C["text"]).pack(anchor="w")
        ctk.CTkLabel(title_box, text="Configure el puerto serial del PIC18F57Q43",
                     font=font("tiny"), text_color=C["muted"]).pack(anchor="w")

        sep(panel).grid(row=1, column=0, sticky="ew", padx=32, pady=8)

        # ----------------------------------------------------------------------
        # Formulario de Conexión (Puerto y Baudrate)
        # ----------------------------------------------------------------------
        form = ctk.CTkFrame(panel, fg_color="transparent")
        form.grid(row=2, column=0, sticky="ew", padx=32, pady=8)
        form.grid_columnconfigure(0, weight=1)
        form.grid_columnconfigure(1, weight=1)

        # Combo: Puerto
        ctk.CTkLabel(form, text="Puerto Serial", font=font("small"),
                     text_color=C["text2"]).grid(row=0, column=0, sticky="w", pady=(0, 4))
        self.port_cb = ctk.CTkComboBox(
            form, values=PORTS, font=font("body"),
            fg_color=C["input"], border_color=C["border"],
            button_color=C["accent"], button_hover_color=C["accent_hover"],
            dropdown_fg_color=C["card"], dropdown_hover_color=C["card_hover"],
            text_color=C["text"], dropdown_text_color=C["text"],
            corner_radius=CORNER, height=42)
        self.port_cb.set(PORTS[0])
        self.port_cb.grid(row=1, column=0, sticky="ew", padx=(0, 8), pady=(0, 16))

        # Combo: Baudrate
        ctk.CTkLabel(form, text="Baudrate", font=font("small"),
                     text_color=C["text2"]).grid(row=0, column=1, sticky="w", pady=(0, 4))
        self.baud_cb = ctk.CTkComboBox(
            form, values=BAUDRATES, font=font("body"),
            fg_color=C["input"], border_color=C["border"],
            button_color=C["accent"], button_hover_color=C["accent_hover"],
            dropdown_fg_color=C["card"], dropdown_hover_color=C["card_hover"],
            text_color=C["text"], dropdown_text_color=C["text"],
            corner_radius=CORNER, height=42)
        self.baud_cb.set("9600")
        self.baud_cb.grid(row=1, column=1, sticky="ew", pady=(0, 16))

        # ----------------------------------------------------------------------
        # Indicadores y Botones
        # ----------------------------------------------------------------------
        led_row = ctk.CTkFrame(panel, fg_color="transparent")
        led_row.grid(row=3, column=0, padx=32, sticky="w")
        ctk.CTkLabel(led_row, text="Estado:", font=font("small"),
                     text_color=C["muted"]).pack(side="left")
        self._led = ctk.CTkFrame(led_row, width=12, height=12,
                                 fg_color=C["red"], corner_radius=6)
        self._led.pack(side="left", padx=6)
        self._status_lbl = ctk.CTkLabel(led_row, text="Desconectado",
                                         font=font("small"), text_color=C["muted"])
        self._status_lbl.pack(side="left")

        sep(panel).grid(row=4, column=0, sticky="ew", padx=32, pady=12)

        btn_row = ctk.CTkFrame(panel, fg_color="transparent")
        btn_row.grid(row=5, column=0, sticky="ew", padx=32, pady=(0, 32))
        btn_row.grid_columnconfigure(0, weight=1)
        btn_row.grid_columnconfigure(1, weight=2)

        ghost_btn(btn_row, "←  Volver", command=self.on_back).grid(
            row=0, column=0, sticky="ew", padx=(0, 8))
        self._connect_btn = accent_btn(btn_row, "🔗  Conectar",
                                        command=self._handle_connect)
        self._connect_btn.grid(row=0, column=1, sticky="ew")

        info = ctk.CTkFrame(self, fg_color="transparent")
        info.grid(row=1, column=0, pady=(16, 32))
        
        for txt in [f"Dispositivo: PIC18F57Q43   •   {APP_VERSION}   •   Protocolo: UART"]:
            ctk.CTkLabel(info, text=txt, font=font("tiny"),
                         text_color=C["muted"]).pack()

    # ========================================================================
    # FUNCIÓN: _handle_connect
    # ========================================================================
    # Intenta realizar la conexión real con el puerto serial seleccionado.
    #
    # Flujo:
    #  1. Extrae puerto y baudrate de la UI.
    #  2. Bloquea el botón e inicia indicador visual.
    #  3. Invoca la conexión en el backend.
    #  4. Si conecta, lanza workspace; si no, restaura UI de error.
    # ========================================================================
    def _handle_connect(self):
        # Variables locales
        selected_port = self.port_cb.get()
        selected_baud = self.baud_cb.get()
        port_to_open = selected_port.split()[0] 

        self._connect_btn.configure(state="disabled", text="Conectando...")
        self._led.configure(fg_color=C["yellow"])
        self._status_lbl.configure(text="Abriendo puerto...", text_color=C["yellow"])

        # Buscar la instancia principal App
        app_instance = self.winfo_toplevel()

        # Intentar conectar
        success = app_instance.backend.connect(port=port_to_open, baudrate=selected_baud)

        if success:
            self._led.configure(fg_color=C["green"])
            self._status_lbl.configure(text="Conectado", text_color=C["green"])
            self.after(300, app_instance._launch_workspace)
        else:
            self._led.configure(fg_color=C["red"])
            self._status_lbl.configure(text="Fallo de conexión", text_color=C["red"])
            self._connect_btn.configure(state="normal", text="🔗  Conectar")


# ============================================================================
# CLASE / FUNCIÓN: Sidebar
# ============================================================================
# Menú lateral de navegación. Solo se instancia DESPUÉS de la conexión.
#
# Flujo:
#  1. Genera botones por cada sección disponible.
#  2. Maneja el estado "Activo" visualmente.
# ============================================================================
class Sidebar(ctk.CTkFrame):

    NAV = [
        ("home",         "🏠", "Inicio"),
        ("dashboard",    "📊", "Dashboard"),
        ("schedule",     "⏰", "Horarios"),
        ("compartments", "💊", "Compartimentos"),
        ("diagnostic",   "🛠️", "Diagnóstico"),
        ("logs",         "📋", "Logs"),
    ]

    def __init__(self, parent, on_navigate):
        super().__init__(parent, fg_color=C["sidebar"], corner_radius=0, width=210)
        self.on_navigate = on_navigate
        self._buttons = {}
        self._active  = None

        self.grid_rowconfigure(2, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self._build()

    # ========================================================================
    # FUNCIÓN: _build (Sidebar)
    # ========================================================================
    def _build(self):
        # ----------------------------------------------------------------------
        # Logo y Título
        # ----------------------------------------------------------------------
        logo_area = ctk.CTkFrame(self, fg_color="transparent")
        logo_area.grid(row=0, column=0, sticky="ew", padx=16, pady=(24, 8))
        logo_area.grid_columnconfigure(0, weight=1)

        pill_bg = ctk.CTkFrame(logo_area, width=44, height=44,
                               fg_color=C["purple"], corner_radius=12)
        pill_bg.grid(row=0, column=0, pady=(0, 6))
        pill_bg.grid_propagate(False)
        ctk.CTkLabel(pill_bg, text="💊", font=ctk.CTkFont(size=22),
                     text_color=C["text"]).place(relx=0.5, rely=0.5, anchor="center")

        ctk.CTkLabel(logo_area, text="Dispensador\nInteligente",
                     font=font("small", True), text_color=C["text"],
                     justify="center").grid(row=1, column=0)

        sep(self).grid(row=1, column=0, sticky="ew", padx=16, pady=8)

        # ----------------------------------------------------------------------
        # Botones de Navegación
        # ----------------------------------------------------------------------
        nav_frame = ctk.CTkFrame(self, fg_color="transparent")
        nav_frame.grid(row=2, column=0, sticky="nsew", padx=8)
        nav_frame.grid_columnconfigure(0, weight=1)

        for idx, (pid, icon, label) in enumerate(self.NAV):
            btn = self._make_nav_btn(nav_frame, pid, icon, label)
            btn.grid(row=idx, column=0, sticky="ew", pady=2)
            self._buttons[pid] = btn

        # ----------------------------------------------------------------------
        # Pie / Status Inferior
        # ----------------------------------------------------------------------
        sep(self).grid(row=3, column=0, sticky="ew", padx=16, pady=(8, 0))

        foot = ctk.CTkFrame(self, fg_color="transparent")
        foot.grid(row=4, column=0, sticky="ew", padx=16, pady=16)

        led_row = ctk.CTkFrame(foot, fg_color="transparent")
        led_row.pack(anchor="w")
        ctk.CTkFrame(led_row, width=10, height=10,
                     fg_color=C["green"], corner_radius=5).pack(side="left", padx=(0, 6))
        ctk.CTkLabel(led_row, text="Conectado", font=font("small"),
                     text_color=C["green"]).pack(side="left")

        ctk.CTkLabel(foot, text=f"{APP_VERSION}  •  PIC18F57Q43",
                     font=font("tiny"), text_color=C["muted"]).pack(anchor="w", pady=(4, 0))

    # ========================================================================
    # FUNCIÓN: _make_nav_btn
    # ========================================================================
    # Crea un botón personalizado para el menú lateral.
    # ========================================================================
    def _make_nav_btn(self, parent, pid, icon, label):
        frame = ctk.CTkFrame(parent, fg_color="transparent",
                             corner_radius=CORNER, height=44, cursor="hand2")
        frame.grid_columnconfigure(2, weight=1)
        frame.grid_propagate(False)

        indicator = ctk.CTkFrame(frame, width=4, height=26,
                                 fg_color="transparent", corner_radius=2)
        indicator.grid(row=0, column=0, padx=(4, 0), pady=9)

        icon_lbl = ctk.CTkLabel(frame, text=icon, font=ctk.CTkFont(size=16),
                                text_color=C["text2"], width=28)
        icon_lbl.grid(row=0, column=1, padx=(6, 4))

        text_lbl = ctk.CTkLabel(frame, text=label, font=font("body"),
                                text_color=C["text2"], anchor="w")
        text_lbl.grid(row=0, column=2, sticky="w")

        frame._indicator = indicator
        frame._icon_lbl  = icon_lbl
        frame._text_lbl  = text_lbl

        def on_click(e=None, p=pid):
            self.on_navigate(p)

        for w in (frame, indicator, icon_lbl, text_lbl):
            w.bind("<Button-1>", on_click)
            w.bind("<Enter>", lambda e, f=frame, p=pid:
                   f.configure(fg_color=C["card_hover"]) if p != self._active else None)
            w.bind("<Leave>", lambda e, f=frame, p=pid:
                   f.configure(fg_color="transparent") if p != self._active else None)

        return frame

    # ========================================================================
    # FUNCIÓN: set_active
    # ========================================================================
    # Resalta visualmente el ítem activo en el sidebar.
    # ========================================================================
    def set_active(self, page_id):
        for pid, frame in self._buttons.items():
            active = pid == page_id
            frame.configure(fg_color=C["card"] if active else "transparent")
            frame._indicator.configure(fg_color=C["purple"] if active else "transparent")
            color = C["text"] if active else C["text2"]
            frame._icon_lbl.configure(text_color=color)
            frame._text_lbl.configure(text_color=color,
                                       font=font("body", bold=active))
        self._active = page_id


# ============================================================================
# CLASE / FUNCIÓN: HomePage
# ============================================================================
# Pantalla de bienvenida dentro del entorno de trabajo.
# Muestra un resumen del estado del sistema post-conexión.
# ============================================================================
class HomePage(ctk.CTkFrame):

    def __init__(self, parent):
        super().__init__(parent, fg_color=C["bg"], corner_radius=0)
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)
        self._build()

    # ========================================================================
    # FUNCIÓN: _build (HomePage)
    # ========================================================================
    def _build(self):
        # ----------------------------------------------------------------------
        # Encabezado
        # ----------------------------------------------------------------------
        hdr = ctk.CTkFrame(self, fg_color="transparent")
        hdr.grid(row=0, column=0, sticky="ew", padx=32, pady=(32, 16))
        ctk.CTkLabel(hdr, text="🏠  Bienvenido al Sistema",
                     font=font("sub", True), text_color=C["text"]).pack(side="left")

        # ----------------------------------------------------------------------
        # Tarjeta Central Info
        # ----------------------------------------------------------------------
        c = card(self)
        c.grid(row=1, column=0, sticky="nsew", padx=32, pady=(0, 32))
        c.grid_columnconfigure(0, weight=1)
        c.grid_rowconfigure(2, weight=1)

        ctk.CTkLabel(c, text="Información general del sistema",
                     font=font("body"), text_color=C["text2"]).grid(
            row=0, column=0, padx=24, pady=(24, 8), sticky="w")

        info_items = [
            ("Estado de conexión" , "✅ Conectado — COM3 @ 9600 bps"),
            ("Dispositivo"        , "PIC18F57Q43"),
            ("Versión"            , APP_VERSION),
            ("Fecha de compilación", APP_BUILD_DATE),
            ("Versión de Python" , "3.9.2")
        ]

        info_grid = ctk.CTkFrame(c, fg_color="transparent")
        info_grid.grid(row=1, column=0, sticky="ew", padx=24, pady=8)
        info_grid.grid_columnconfigure(1, weight=1)

        for i, (key, val) in enumerate(info_items):
            ctk.CTkLabel(info_grid, text=f"{key}:", font=font("small"),
                         text_color=C["muted"], width=180, anchor="w").grid(
                row=i, column=0, sticky="w", pady=4)
            color = C["green"] if "Conectado" in val else (C["yellow"] if "bajo" in val else C["text"])
            ctk.CTkLabel(info_grid, text=val, font=font("small", True),
                         text_color=color, anchor="w").grid(
                row=i, column=1, sticky="w", padx=8, pady=4)

        tip = ctk.CTkFrame(c, fg_color=dim(C["accent"], 0.12), corner_radius=8)
        tip.grid(row=2, column=0, sticky="sew", padx=24, pady=24)
        ctk.CTkLabel(tip, text="💡  Use el menú lateral para navegar entre las secciones del sistema.",
                     font=font("small"), text_color=C["accent"]).pack(padx=16, pady=12)


# ============================================================================
# CLASE / FUNCIÓN: DashboardPage
# ============================================================================
# Dashboard de supervisión con tarjetas KPI, barras de progreso y reloj.
#
# Flujo:
#  1. Construye el reloj y encabezado superior.
#  2. Genera los KPIs (Estado PIC, Horarios).
#  3. Construye el layout de niveles de compartimentos.
#  4. Construye el visualizador de la próxima dosis.
#  5. Inicia el loop que refresca la hora y el estado de la comunicación.
# ============================================================================
class DashboardPage(ctk.CTkFrame):

    def __init__(self, parent):
        super().__init__(parent, fg_color=C["bg"], corner_radius=0)
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)
        self._build()
        self._tick_clock()

    # ========================================================================
    # FUNCIÓN: _build (DashboardPage)
    # ========================================================================
    def _build(self):
        hdr = ctk.CTkFrame(self, fg_color="transparent")
        hdr.grid(row=0, column=0, sticky="ew", padx=32, pady=(32, 16))
        hdr.grid_columnconfigure(0, weight=1)

        ctk.CTkLabel(hdr, text="📊  Dashboard de Supervisión",
                     font=font("sub", True), text_color=C["text"]).grid(
            row=0, column=0, sticky="w")

        clock_card = ctk.CTkFrame(hdr, fg_color=C["card"], corner_radius=10,
                                  border_width=1, border_color=C["border"])
        clock_card.grid(row=0, column=1, sticky="e")
        self._clock = ctk.CTkLabel(clock_card, text="--:--:--",
                                    font=font("body", True), text_color=C["accent"])
        self._clock.pack(padx=16, pady=6)

        scroll = ctk.CTkScrollableFrame(self, fg_color=C["bg"],
                                         scrollbar_button_color=C["border"],
                                         scrollbar_button_hover_color=C["accent"])
        scroll.grid(row=1, column=0, sticky="nsew", padx=32)
        scroll.grid_columnconfigure(0, weight=1)

        self._build_kpis(scroll)
        self._build_levels(scroll)
        self._build_next_dose(scroll)

    # ========================================================================
    # FUNCIÓN: _build_kpis
    # ========================================================================
    def _build_kpis(self, parent):
        row = ctk.CTkFrame(parent, fg_color="transparent")
        row.grid(row=0, column=0, sticky="ew", pady=(0, 16))
        row.grid_columnconfigure(0, weight=1)
        row.grid_columnconfigure(1, weight=1)

        # ----------------------------------------------------------------------
        # KPI 1: ESTADO PIC 
        # ----------------------------------------------------------------------
        c1 = card(row)
        c1.grid(row=0, column=0, sticky="ew", padx=(0, 6))
        
        self.kpi_pic_strip = ctk.CTkFrame(c1, height=4, fg_color=C["green"], corner_radius=0)
        self.kpi_pic_strip.pack(fill="x")
        
        body1 = ctk.CTkFrame(c1, fg_color="transparent")
        body1.pack(fill="x", padx=16, pady=12)
        ctk.CTkLabel(body1, text="🖥️", font=ctk.CTkFont(size=18)).pack(side="left")
        txt_box1 = ctk.CTkFrame(body1, fg_color="transparent")
        txt_box1.pack(side="left", padx=8)
        ctk.CTkLabel(txt_box1, text="Estado PIC", font=font("tiny"), text_color=C["muted"]).pack(anchor="w")
        
        self.kpi_pic = ctk.CTkLabel(txt_box1, text="Conectado", font=font("small", True), text_color=C["green"])
        self.kpi_pic.pack(anchor="w")
       
        # ----------------------------------------------------------------------
        # KPI 2: HORARIOS CONFIGURADOS
        # ----------------------------------------------------------------------
        c2 = card(row)
        c2.grid(row=0, column=1, sticky="ew", padx=6)
        ctk.CTkFrame(c2, height=4, fg_color=C["orange"], corner_radius=0).pack(fill="x")
        body2 = ctk.CTkFrame(c2, fg_color="transparent")
        body2.pack(fill="x", padx=16, pady=12)
        ctk.CTkLabel(body2, text="🔔", font=ctk.CTkFont(size=18)).pack(side="left")
        txt_box2 = ctk.CTkFrame(body2, fg_color="transparent")
        txt_box2.pack(side="left", padx=8)
        ctk.CTkLabel(txt_box2, text="Horarios", font=font("tiny"), text_color=C["muted"]).pack(anchor="w")
        
        self.kpi_horarios = ctk.CTkLabel(txt_box2, text="0 Programados", font=font("small", True), text_color=C["orange"])
        self.kpi_horarios.pack(anchor="w")

    # ========================================================================
    # FUNCIÓN: _build_levels
    # ========================================================================
    # Construye el panel visual con barras de progreso de inventario.
    # ========================================================================
    def _build_levels(self, parent):
        c = card(parent)
        c.grid(row=1, column=0, sticky="ew", pady=(0, 16))
        c.grid_columnconfigure(0, weight=1)

        hdr = ctk.CTkFrame(c, fg_color="transparent")
        hdr.grid(row=0, column=0, sticky="ew", padx=20, pady=(16, 8))
        ctk.CTkLabel(hdr, text="💊  Niveles de Compartimentos", font=font("body", True), text_color=C["text"]).pack(side="left")
        sep(c).grid(row=1, column=0, sticky="ew", padx=20)

        levels = [
            ("Comp. 1", NOMBRE_PASTILLA_COMPARTMENTOS[0], C["accent"]),
            ("Comp. 2", NOMBRE_PASTILLA_COMPARTMENTOS[1], C["green"] ),
            ("Comp. 3", NOMBRE_PASTILLA_COMPARTMENTOS[2], C["yellow"]),
            ("Comp. 4", NOMBRE_PASTILLA_COMPARTMENTOS[3], C["red"]   ),
        ]

        container = ctk.CTkFrame(c, fg_color="transparent")
        container.grid(row=2, column=0, sticky="ew", padx=20, pady=16)
        container.grid_columnconfigure(0, weight=1)

        self.dash_bars = []
        self.dash_pcts = []

        for idx, (comp, med, color) in enumerate(levels):
            r = ctk.CTkFrame(container, fg_color="transparent")
            r.grid(row=idx * 2, column=0, sticky="ew", pady=(4, 0))
            r.grid_columnconfigure(1, weight=1)

            ctk.CTkLabel(r, text=comp, font=font("small", True), text_color=C["text"], width=70).grid(row=0, column=0, sticky="w")
            ctk.CTkLabel(r, text=med, font=font("tiny"), text_color=C["text2"]).grid(row=0, column=1, sticky="w", padx=8)
            
            pct_lbl = ctk.CTkLabel(r, text="0%", font=font("small", True), text_color=color)
            pct_lbl.grid(row=0, column=2, sticky="e")
            self.dash_pcts.append(pct_lbl)

            bar = ctk.CTkProgressBar(container, progress_color=color, fg_color=C["border"], corner_radius=5, height=8)
            bar.set(0.0)
            bar.grid(row=idx * 2 + 1, column=0, sticky="ew", pady=(4, 8))
            self.dash_bars.append(bar)

    # ========================================================================
    # FUNCIÓN: _build_next_dose
    # ========================================================================
    # Construye el panel con la información de la siguiente alarma.
    # ========================================================================
    def _build_next_dose(self, parent):
        c = card(parent)
        c.grid(row=3, column=0, sticky="ew", pady=(0, 32))
        c.grid_columnconfigure(0, weight=1)

        hdr = ctk.CTkFrame(c, fg_color="transparent")
        hdr.grid(row=0, column=0, sticky="ew", padx=20, pady=(16, 8))
        ctk.CTkLabel(hdr, text="⏰  Próxima Dosis Programada",
                     font=font("body", True), text_color=C["text"]).pack(side="left")
        sep(c).grid(row=1, column=0, sticky="ew", padx=20)

        grid = ctk.CTkFrame(c, fg_color="transparent")
        grid.grid(row=2, column=0, sticky="ew", padx=20, pady=16)
        for i in range(4): grid.grid_columnconfigure(i, weight=1)

        p1 = ctk.CTkFrame(grid, fg_color=C["input"], corner_radius=8); p1.grid(row=0, column=0, padx=4, sticky="ew")
        ctk.CTkLabel(p1, text="Medicamento", font=font("tiny"), text_color=C["muted"]).pack(padx=16, pady=(8, 0))
        self.nxt_med = ctk.CTkLabel(p1, text="Ninguno", font=font("small", True), text_color=C["accent"])
        self.nxt_med.pack(padx=16, pady=(0, 8))

        p2 = ctk.CTkFrame(grid, fg_color=C["input"], corner_radius=8); p2.grid(row=0, column=1, padx=4, sticky="ew")
        ctk.CTkLabel(p2, text="Compartimento", font=font("tiny"), text_color=C["muted"]).pack(padx=16, pady=(8, 0))
        self.nxt_comp = ctk.CTkLabel(p2, text="--", font=font("small", True), text_color=C["accent"])
        self.nxt_comp.pack(padx=16, pady=(0, 8))

        p3 = ctk.CTkFrame(grid, fg_color=C["input"], corner_radius=8); p3.grid(row=0, column=2, padx=4, sticky="ew")
        ctk.CTkLabel(p3, text="Hora", font=font("tiny"), text_color=C["muted"]).pack(padx=16, pady=(8, 0))
        self.nxt_time = ctk.CTkLabel(p3, text="--:--", font=font("small", True), text_color=C["accent"])
        self.nxt_time.pack(padx=16, pady=(0, 8))

        p4 = ctk.CTkFrame(grid, fg_color=C["input"], corner_radius=8); p4.grid(row=0, column=3, padx=4, sticky="ew")
        ctk.CTkLabel(p4, text="Cantidad", font=font("tiny"), text_color=C["muted"]).pack(padx=16, pady=(8, 0))
        self.nxt_qty = ctk.CTkLabel(p4, text="--", font=font("small", True), text_color=C["accent"])
        self.nxt_qty.pack(padx=16, pady=(0, 8))

    # ========================================================================
    # FUNCIÓN: _tick_clock
    # ========================================================================
    def _tick_clock(self):
        self._clock.configure(text=datetime.now().strftime("%H:%M:%S"))
        main_app = self.winfo_toplevel()
        
        if hasattr(main_app, "backend") and hasattr(self, "kpi_pic") and hasattr(self, "kpi_pic_strip"):
            if main_app.backend.is_connected:
                self.kpi_pic.configure(text="Conectado", text_color=C["green"])
                self.kpi_pic_strip.configure(fg_color=C["green"])
                
                # 📢 LA CONDICIÓN CLAVE:
                # Solo pedimos los 40 bytes de memoria general si la pestaña 
                # activa NO es la de diagnóstico.
                if hasattr(main_app, "_sidebar") and main_app._sidebar._active != "diagnostic":
                    main_app.backend.request_memory_data()
                    
            else:
                self.kpi_pic.configure(text="Desconectado", text_color=C["red"])
                self.kpi_pic_strip.configure(fg_color=C["red"])
                
        self.after(2000, self._tick_clock)

# ============================================================================
# CLASE / FUNCIÓN: CompartmentsPage
# ============================================================================
# Grid 2×2 de tarjetas con modal de RECARGA INCREMENTAL inteligente
# ============================================================================
class CompartmentsPage(ctk.CTkFrame):

    def __init__(self, parent):
        super().__init__(parent, fg_color=C["bg"], corner_radius=0)
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)
        self._build()

    # ========================================================================
    # FUNCIÓN: _build (CompartmentsPage)
    # ========================================================================
    def _build(self):
        hdr = ctk.CTkFrame(self, fg_color="transparent")
        hdr.grid(row=0, column=0, sticky="ew", padx=32, pady=(32, 16))
        ctk.CTkLabel(hdr, text="💊  Compartimentos",
                     font=font("sub", True), text_color=C["text"]).pack(side="left")

        self.grid_rowconfigure(1, weight=0)
        self.grid_rowconfigure(2, weight=1)

        grid = ctk.CTkFrame(self, fg_color="transparent")
        grid.grid(row=1, column=0, sticky="ew", padx=32, pady=0) 
        
        for i in range(2):
            grid.grid_columnconfigure(i, weight=1)
            grid.grid_rowconfigure(i, weight=0) 

        base_configs = [
            {"id": "1", "color": C["accent"]},
            {"id": "2", "color": C["green"]},
            {"id": "3", "color": C["yellow"]},
            {"id": "4", "color": C["red"]}
        ]

        self.tarjetas_referencias = []

        for idx, config in enumerate(base_configs):
            r, cl = divmod(idx, 2)
            px = (0, 8) if cl == 0 else (8, 0)
            py = (0, 8) if r  == 0 else (8, 0)
            
            card_widget = self._crear_tarjeta_fija(grid, config)
            card_widget.grid(row=r, column=cl, sticky="nsew", padx=px, pady=py)

        spacer = ctk.CTkFrame(self, fg_color="transparent", height=1)
        spacer.grid(row=2, column=0, sticky="nsew")

    # ========================================================================
    # FUNCIÓN: _crear_tarjeta_fija
    # ========================================================================
    def _crear_tarjeta_fija(self, parent, d):
        c = ctk.CTkFrame(parent, fg_color=C["card"], corner_radius=CORNER,
                         border_width=2, border_color=d["color"])
        c.grid_columnconfigure(0, weight=1)

        header = ctk.CTkFrame(c, fg_color=dim(d["color"], 0.15), corner_radius=CORNER)
        header.grid(row=0, column=0, sticky="ew")
        header.grid_columnconfigure(0, weight=1)

        inner = ctk.CTkFrame(header, fg_color="transparent")
        inner.grid(row=0, column=0, padx=16, pady=14, sticky="ew")
        inner.grid_columnconfigure(0, weight=1)

        comp_id = int(d['id'])
        comp_nombre = NOMBRE_PASTILLA_COMPARTMENTOS[comp_id - 1]

        ctk.CTkLabel(inner, text=f"Compartimento {comp_id}",
                     font=font("small", True), text_color=C["text2"]).grid(row=0, column=0, sticky="w")
        
        lbl_med = ctk.CTkLabel(inner, text=comp_nombre, 
                              font=font("body", True), text_color=d["color"])
        lbl_med.grid(row=1, column=0, sticky="w", pady=(4, 0))

        sep(c).grid(row=1, column=0, sticky="ew")

        body = ctk.CTkFrame(c, fg_color="transparent")
        body.grid(row=2, column=0, sticky="ew", padx=16, pady=12) 
        body.grid_columnconfigure(0, weight=1)

        lvl_row = ctk.CTkFrame(body, fg_color="transparent")
        lvl_row.grid(row=0, column=0, sticky="ew")
        lvl_row.grid_columnconfigure(0, weight=1)
        ctk.CTkLabel(lvl_row, text="Cantidad de Pastillas", font=font("tiny"), text_color=C["muted"]).grid(row=0, column=0, sticky="w")
        
        lbl_qty_fraction = ctk.CTkLabel(lvl_row, text=f"0 / {CAPACITY_MAX}", font=font("tiny", True), text_color=C["text2"])
        lbl_qty_fraction.grid(row=0, column=1, sticky="e")

        progress_bar = ctk.CTkProgressBar(body, progress_color=d["color"], fg_color=C["border"], corner_radius=5, height=10)
        progress_bar.set(0.0)
        progress_bar.grid(row=1, column=0, sticky="ew", pady=(6, 0))

        sep(c).grid(row=3, column=0, sticky="ew")
        btn_row = ctk.CTkFrame(c, fg_color="transparent")
        btn_row.grid(row=4, column=0, sticky="ew", padx=16, pady=10)
        btn_row.grid_columnconfigure(0, weight=1)

        btn_edit = ghost_btn(btn_row, "📥 Recargar", 
                             command=lambda cid=comp_id, nom=comp_nombre, col=d["color"]: self._abrir_modal_editar(cid, nom, col))
        btn_edit.grid(row=0, column=0, sticky="ew")

        self.tarjetas_referencias.append({
            "qty_lbl"  : lbl_qty_fraction,
            "bar"      : progress_bar,
            "border_c" : c,
            "qty_actual": 0
        })
        return c

    # ========================================================================
    # FUNCIÓN: actualizar_compartimentos
    # ========================================================================
    def actualizar_compartimentos(self, cantidades):
        self.after(0, self._aplicar_valores, cantidades)

    def _aplicar_valores(self, cantidades):
        for i in range(4):
            qty = cantidades[i]
            refs = self.tarjetas_referencias[i]
            refs["qty_actual"] = qty 
            
            porcentaje = min(1.0, max(0.0, qty / CAPACITY_MAX))
            
            refs["qty_lbl"].configure(text=f"{qty} / {CAPACITY_MAX}")
            refs["bar"].set(porcentaje)

            if qty == 0:
                refs["border_c"].configure(border_color=C["red"])
            elif qty <= 3:
                refs["border_c"].configure(border_color=C["yellow"])
            else:
                colors = [C["accent"], C["green"], C["yellow"], C["red"]]
                refs["border_c"].configure(border_color=colors[i])

    # ========================================================================
    # FUNCIÓN: _abrir_modal_editar
    # ========================================================================
    def _abrir_modal_editar(self, comp_id, comp_nombre, color_base):
        modal = ctk.CTkToplevel(self)
        modal.title(f"Recargar Compartimento {comp_id}")
        modal.geometry("360x420")
        modal.resizable(False, False)
        modal.attributes("-topmost", True)
        modal.grab_set()
        modal.configure(fg_color=C["bg"])
        
        modal.update_idletasks()
        x = (modal.winfo_screenwidth() - 360) // 2
        y = (modal.winfo_screenheight() - 420) // 2
        modal.geometry(f"+{x}+{y}")
        
        ctk.CTkLabel(modal, text="📥 Recargar Inventario", font=font("sub", True), text_color=C["text"]).pack(pady=(20, 10))
        
        form = ctk.CTkFrame(modal, fg_color="transparent")
        form.pack(padx=30, pady=10, fill="both", expand=True)
        
        ctk.CTkLabel(form, text=f"Compartimento {comp_id} - {comp_nombre}", font=font("small", True), text_color=color_base).pack(anchor="w", pady=(0, 15))
        
        cantidad_actual_memoria = self.tarjetas_referencias[comp_id - 1]["qty_actual"]
        espacio_disponible = CAPACITY_MAX - cantidad_actual_memoria
        
        ctk.CTkLabel(form, text=f"Inventario Actual: {cantidad_actual_memoria} / {CAPACITY_MAX} pastillas", font=font("small"), text_color=C["text2"]).pack(anchor="w", pady=(0, 10))
        
        # Generamos la lista del 0 hasta el espacio disponible
        if espacio_disponible > 0:
            ctk.CTkLabel(form, text=f"¿Cuántas pastillas nuevas vas a ingresar? (Máx {espacio_disponible}):", font=font("small"), text_color=C["text2"]).pack(anchor="w", pady=(5, 5))
            
            opciones_cantidad = [str(i) for i in range(0, espacio_disponible + 1)]
            qty_cb = ctk.CTkComboBox(form, values=opciones_cantidad, button_color=color_base, button_hover_color=color_base)
            qty_cb.pack(fill="x", pady=(0, 20))
            qty_cb.set("0")
        else:
            ctk.CTkLabel(form, text="¡El compartimento está totalmente lleno!", font=font("small", True), text_color=C["green"]).pack(anchor="w", pady=(15, 20))
            qty_cb = None
        
        def guardar_recarga():
            if qty_cb is None:
                modal.destroy()
                return
            try:
                # 📢 ¡AQUÍ ESTÁ LA CORRECCIÓN! 
                # Tomamos la cantidad PURA que el usuario seleccionó de la lista
                cantidad_a_agregar = int(qty_cb.get())
                
                if cantidad_a_agregar < 0 or cantidad_a_agregar > espacio_disponible:
                    self._mostrar_alerta("Límite Excedido", f"Solo puedes agregar hasta {espacio_disponible} pastillas en este momento.")
                    return
                
                # Enviamos el número limpio. El PIC se encargará de sumarlo.
                main_app = self.winfo_toplevel()
                if hasattr(main_app, "backend") and main_app.backend.is_connected:
                    main_app.backend.write_pillbox_data(cantidad_a_agregar, comp_id)
                
                modal.destroy()
            except ValueError:
                self._mostrar_alerta("Entrada Inválida", "Por favor, ingrese únicamente números enteros.")

        

        if espacio_disponible > 0:
            accent_btn(modal, "💾  Confirmar Recarga", command=guardar_recarga, fg_color=color_base).pack(fill="x", padx=30, pady=(10, 10))
            
        

    # ========================================================================
    # FUNCIÓN: _mostrar_alerta
    # ========================================================================
    def _mostrar_alerta(self, titulo, mensaje):
        alerta = ctk.CTkToplevel(self)
        alerta.title(titulo)
        alerta.geometry("350x180")
        alerta.resizable(False, False)
        alerta.attributes("-topmost", True)
        alerta.grab_set()
        alerta.configure(fg_color=C["bg"])
        
        alerta.update_idletasks()
        x = (alerta.winfo_screenwidth() - 350) // 2
        y = (alerta.winfo_screenheight() - 180) // 2
        alerta.geometry(f"+{x}+{y}")
        
        ctk.CTkLabel(alerta, text="⚠️", font=ctk.CTkFont(size=30)).pack(pady=(20, 5))
        ctk.CTkLabel(alerta, text=mensaje, font=font("body"), text_color=C["text"], wraplength=300).pack()
        ghost_btn(alerta, "Aceptar", command=alerta.destroy, width=120).pack(pady=(15, 0))
# ============================================================================
# CLASE / FUNCIÓN: SchedulePage
# ============================================================================
# Tabla de horarios fija con asignación automática del siguiente espacio libre.
# ============================================================================
class SchedulePage(ctk.CTkFrame):

    def __init__(self, parent):
        super().__init__(parent, fg_color=C["bg"], corner_radius=0)
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)
        
        # Guardamos el estado local para saber qué espacios están libres
        self.estado_horarios = [{"estado": "Espacio Vacío"} for _ in range(6)]
        
        self._build()

    # ========================================================================
    # FUNCIÓN: _build (SchedulePage)
    # ========================================================================
    def _build(self):
        hdr = ctk.CTkFrame(self, fg_color="transparent")
        hdr.grid(row=0, column=0, sticky="ew", padx=32, pady=(32, 16))
        ctk.CTkLabel(hdr, text="⏰  Horarios de Dispensación",
                     font=font("sub", True), text_color=C["text"]).pack(side="left")
        
        accent_btn(hdr, "+ Nuevo Horario", height=36,
                   font=font("small", True), width=160,
                   command=self._abrir_modal_horario).pack(side="right")

        tbl = card(self)
        tbl.grid(row=1, column=0, sticky="nsew", padx=32, pady=(0, 32))
        tbl.grid_rowconfigure(2, weight=1)
        tbl.grid_columnconfigure(0, weight=1)

        cols = ["Alarma", "Medicamento", "Compartimento", "Hora Programada", "Estado Actual"]
        col_w = [2, 4, 3, 3, 3]

        hdr_row = ctk.CTkFrame(tbl, fg_color=C["input"], corner_radius=0, height=40)
        hdr_row.grid(row=0, column=0, sticky="ew", padx=16, pady=(16, 0))
        hdr_row.grid_propagate(False)
        
        for i, w in enumerate(col_w):
            hdr_row.grid_columnconfigure(int(i), weight=int(w))
            
        for i, col in enumerate(cols):
            ctk.CTkLabel(hdr_row, text=col, font=font("medium", True),
                         text_color=C["text"], anchor="w").grid(
                row=0, column=i, sticky="w", padx=10, pady=10)

        scroll = ctk.CTkScrollableFrame(tbl, fg_color=C["card"],
                                             scrollbar_button_color=C["border"],
                                             scrollbar_button_hover_color=C["accent"])
        scroll.grid(row=2, column=0, sticky="nsew", padx=16, pady=(0, 16))
        scroll.grid_columnconfigure(0, weight=1)

        # Filas Fijas
        self.filas_labels = [] 

        for idx in range(6):
            row_bg = C["card"] if idx % 2 == 0 else C["input"]
            row = ctk.CTkFrame(scroll, fg_color=row_bg, corner_radius=8, height=48)
            row.grid(row=idx, column=0, sticky="ew", pady=2)
            row.grid_propagate(False)
            
            for i, w in enumerate(col_w):
                row.grid_columnconfigure(int(i), weight=int(w))

            lbl_id     = ctk.CTkLabel(row, text=f"Horario {idx + 1}", font=font("small", True), text_color=C["muted"], anchor="w")
            lbl_med    = ctk.CTkLabel(row, text="---", font=font("small"), text_color=C["muted"], anchor="w")
            lbl_comp   = ctk.CTkLabel(row, text="---", font=font("small"), text_color=C["muted"], anchor="w")
            lbl_time   = ctk.CTkLabel(row, text="--:--", font=font("small"), text_color=C["muted"], anchor="w")
            lbl_estado = ctk.CTkLabel(row, text="Espacio Vacío", font=font("small", True), text_color=C["muted"], anchor="w")

            lbl_id.grid(row=0, column=0, sticky="w", padx=10)
            lbl_med.grid(row=0, column=1, sticky="w", padx=10)
            lbl_comp.grid(row=0, column=2, sticky="w", padx=10)
            lbl_time.grid(row=0, column=3, sticky="w", padx=10)
            lbl_estado.grid(row=0, column=4, sticky="w", padx=10)

            self.filas_labels.append({
                "med": lbl_med,
                "comp": lbl_comp,
                "time": lbl_time,
                "estado": lbl_estado
            })

    # ========================================================================
    # FUNCIÓN: actualizar_tabla
    # ========================================================================
    def actualizar_tabla(self, lista_horarios):
        # Guardamos la lista en la memoria de la clase para saber qué IDs están libres
        self.estado_horarios = lista_horarios
        self.after(0, self._aplicar_cambios_texto, lista_horarios)

    def _aplicar_cambios_texto(self, lista_horarios):
        for idx, datos in enumerate(lista_horarios):
            if idx >= len(self.filas_labels): break 
            widgets = self.filas_labels[idx]
            
            if datos["estado"] == "Espacio Vacío":
                color_med, color_comp, color_time, color_est = C["muted"], C["muted"], C["muted"], C["muted"]
            else:
                color_med  = C["text"]
                color_comp = C["accent"]
                color_time = C["text"]
                color_est  = C["green"] if datos["estado"] == "Ejecutado" else C["text2"]

            widgets["med"].configure(text=datos["med"], text_color=color_med)
            widgets["comp"].configure(text=datos["comp"], text_color=color_comp)
            widgets["time"].configure(text=datos["time"], text_color=color_time)
            widgets["estado"].configure(text=datos["estado"], text_color=color_est)

    # ========================================================================
    # FUNCIÓN: _abrir_modal_horario
    # ========================================================================
    def _abrir_modal_horario(self):
        # 1. Búsqueda automática del primer ID libre (1 al 6)
        siguiente_id = None
        for idx, horario in enumerate(self.estado_horarios):
            if horario.get("estado") == "Espacio Vacío":
                siguiente_id = idx + 1
                break
                
        # 2. Seguro contra memoria llena
        if siguiente_id is None:
            self._mostrar_alerta("Memoria Llena", "Ya se han programado los 6 horarios máximos permitidos.")
            return

        # 3. Construcción del modal
        modal = ctk.CTkToplevel(self)
        modal.title("Configurar Horario")
        modal.geometry("400x420")
        modal.resizable(False, False)
        modal.attributes("-topmost", True)
        modal.grab_set() 
        modal.configure(fg_color=C["bg"])
        
        modal.update_idletasks()
        x = (modal.winfo_screenwidth() - 400) // 2
        y = (modal.winfo_screenheight() - 420) // 2
        modal.geometry(f"+{x}+{y}")
        
        ctk.CTkLabel(modal, text="⚙️ Nuevo Horario", font=font("sub", True), text_color=C["text"]).pack(pady=(20, 10))
        
        form = ctk.CTkFrame(modal, fg_color="transparent")
        form.pack(padx=30, pady=10, fill="both", expand=True)
        
        # Etiqueta estática con el ID auto-asignado en lugar del SegmentedButton
        ctk.CTkLabel(form, text="Horario Asignado Automáticamente:", font=font("small"), text_color=C["text2"]).pack(anchor="w", pady=(10, 0))
        ctk.CTkLabel(form, text=f"Espacio #{siguiente_id}", font=font("title", True), text_color=C["accent"]).pack(anchor="w", pady=(0, 15))
        
        time_frame = ctk.CTkFrame(form, fg_color="transparent")
        time_frame.pack(fill="x", pady=10)
        time_frame.grid_columnconfigure(0, weight=1)
        time_frame.grid_columnconfigure(1, weight=1)
        
        ctk.CTkLabel(time_frame, text="Hora (0-23):", font=font("small"), text_color=C["text2"]).grid(row=0, column=0, sticky="w")
        hora_cb = ctk.CTkComboBox(time_frame, values=[f"{i:02d}" for i in range(24)], width=120)
        hora_cb.grid(row=1, column=0, sticky="w")
        hora_cb.set("08")
        
        ctk.CTkLabel(time_frame, text="Minuto (0-59):", font=font("small"), text_color=C["text2"]).grid(row=0, column=1, sticky="w", padx=(10,0))
        min_cb = ctk.CTkComboBox(time_frame, values=[f"{i:02d}" for i in range(60)], width=120)
        min_cb.grid(row=1, column=1, sticky="w", padx=(10,0))
        min_cb.set("00")
        
        ctk.CTkLabel(form, text="Medicamento / Compartimento:", font=font("small"), text_color=C["text2"]).pack(anchor="w", pady=(15, 0))
        comp_opts = [
            "1 - " + NOMBRE_PASTILLA_COMPARTMENTOS[0], 
            "2 - " + NOMBRE_PASTILLA_COMPARTMENTOS[1], 
            "3 - " + NOMBRE_PASTILLA_COMPARTMENTOS[2], 
            "4 - " + NOMBRE_PASTILLA_COMPARTMENTOS[3]
        ]
        comp_cb = ctk.CTkComboBox(form, values=comp_opts)
        comp_cb.pack(fill="x", pady=(5, 20))
        comp_cb.set(comp_opts[0])
        
        def guardar():
            try:
                h_hora = int(hora_cb.get())
                h_min = int(min_cb.get())
                h_comp = int(comp_cb.get().split(" - ")[0])
                
                main_app = self.winfo_toplevel()
                if hasattr(main_app, "backend") and main_app.backend.is_connected:
                    main_app.backend.write_schedule_data(siguiente_id, h_hora, h_min, h_comp)
                
                modal.destroy()
            except ValueError:
                pass

        accent_btn(modal, "💾  Guardar en el PIC", command=guardar).pack(fill="x", padx=30, pady=(10, 20))

    # ========================================================================
    # FUNCIÓN: _mostrar_alerta
    # ========================================================================
    def _mostrar_alerta(self, titulo, mensaje):
        alerta = ctk.CTkToplevel(self)
        alerta.title(titulo)
        alerta.geometry("350x180")
        alerta.resizable(False, False)
        alerta.attributes("-topmost", True)
        alerta.grab_set()
        alerta.configure(fg_color=C["bg"])
        
        alerta.update_idletasks()
        x = (alerta.winfo_screenwidth() - 350) // 2
        y = (alerta.winfo_screenheight() - 180) // 2
        alerta.geometry(f"+{x}+{y}")
        
        ctk.CTkLabel(alerta, text="⚠️", font=ctk.CTkFont(size=30)).pack(pady=(20, 5))
        ctk.CTkLabel(alerta, text=mensaje, font=font("body"), text_color=C["text"], wraplength=300).pack()
        ghost_btn(alerta, "Aceptar", command=alerta.destroy, width=120).pack(pady=(15, 0))
# ============================================================================
# CLASE / FUNCIÓN: LogsPage
# ============================================================================
# Consola de eventos simulados con marcas de tiempo e inyección de texto.
# ============================================================================
class LogsPage(ctk.CTkFrame):

    def __init__(self, parent):
        super().__init__(parent, fg_color=C["bg"], corner_radius=0)
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)
        self._build()

    # ========================================================================
    # FUNCIÓN: _build (LogsPage)
    # ========================================================================
    def _build(self):
        hdr = ctk.CTkFrame(self, fg_color="transparent")
        hdr.grid(row=0, column=0, sticky="ew", padx=32, pady=(32, 16))
        ctk.CTkLabel(hdr, text="📋  Registro de Eventos",
                     font=font("sub", True), text_color=C["text"]).pack(side="left")

        btn_row = ctk.CTkFrame(hdr, fg_color="transparent")
        btn_row.pack(side="right")
        ghost_btn(btn_row, "📥 Exportar CSV").pack(side="left", padx=(0, 8))
        accent_btn(btn_row, "🗑️ Limpiar", color=C["red"],
                   height=36, font=font("small", True), width=120).pack(side="left")

        console = card(self)
        console.grid(row=1, column=0, sticky="nsew", padx=32, pady=(0, 32))
        console.grid_rowconfigure(0, weight=1)
        console.grid_columnconfigure(0, weight=1)

        self.term = ctk.CTkTextbox(console, fg_color=C["terminal"],
                               text_color=C["term_text"],
                               font=ctk.CTkFont(family="Consolas", size=13),
                               corner_radius=8,
                               scrollbar_button_color=C["border"])
        self.term.grid(row=0, column=0, sticky="nsew", padx=16, pady=16)

# ============================================================================
# CLASE / FUNCIÓN: DiagnosticPage
# ============================================================================
class DiagnosticPage(ctk.CTkFrame):
    def __init__(self, parent):
        super().__init__(parent, fg_color=C["bg"], corner_radius=0)
        self.grid_columnconfigure(0, weight=1)
        self.grid_rowconfigure(1, weight=1)
        self._build()
        self._tick()

    def _build(self):
        hdr = ctk.CTkFrame(self, fg_color="transparent")
        hdr.grid(row=0, column=0, sticky="ew", padx=32, pady=(32, 16))
        ctk.CTkLabel(hdr, text="🛠️  Diagnóstico de Sensores", font=font("sub", True), text_color=C["text"]).pack(side="left")

        panel = card(self)
        panel.grid(row=1, column=0, sticky="nsew", padx=32, pady=(0, 32))
        panel.grid_columnconfigure((0,1), weight=1)

        # Tarjetas CNY70
        ctk.CTkLabel(panel, text="Sensores CNY70 (Ópticos Reflexivos)", font=font("body", True), text_color=C["text"]).grid(row=0, column=0, columnspan=2, pady=20)
        
        self.lbl_cny = []
        for i in range(4):
            r, c = divmod(i, 2)
            crd = ctk.CTkFrame(panel, fg_color=C["input"], corner_radius=8)
            crd.grid(row=r+1, column=c, padx=10, pady=10, sticky="ew")
            ctk.CTkLabel(crd, text=f"CNY70 N° {i+1}", font=font("small"), text_color=C["muted"]).pack(pady=(10,0))
            lbl = ctk.CTkLabel(crd, text="--", font=font("title", True), text_color=C["accent"])
            lbl.pack(pady=(0,10))
            self.lbl_cny.append(lbl)

        sep(panel).grid(row=3, column=0, columnspan=2, sticky="ew", padx=20, pady=20)

        # Tarjeta Sensor IR
        ctk.CTkLabel(panel, text="Sensor Infrarrojo (Vaso/Presencia)", font=font("body", True), text_color=C["text"]).grid(row=4, column=0, columnspan=2)
        ir_crd = ctk.CTkFrame(panel, fg_color=C["input"], corner_radius=8)
        ir_crd.grid(row=5, column=0, columnspan=2, padx=10, pady=10)
        self.lbl_ir = ctk.CTkLabel(ir_crd, text="--", font=font("title", True), text_color=C["purple"])
        self.lbl_ir.pack(padx=40, pady=15)

    def _tick(self):
        main_app = self.winfo_toplevel()
        # SOLO solicita datos si el usuario está viendo esta pantalla
        if hasattr(main_app, "_sidebar") and main_app._sidebar._active == "diagnostic":
            if hasattr(main_app, "backend") and main_app.backend.is_connected:
                main_app.backend.request_diagnostic_data()
                
        self.after(500, self._tick) # Pide datos cada medio segundo

    def actualizar_sensores(self, c1, c2, c3, c4, ir):
        self.after(0, self._aplicar_datos, c1, c2, c3, c4, ir)
        
    def _aplicar_datos(self, c1, c2, c3, c4, ir):
        valores = [c1, c2, c3, c4]
        for idx, lbl in enumerate(self.lbl_cny):
            lbl.configure(text=str(valores[idx]))
        
        self.lbl_ir.configure(text="ACTIVO" if ir == 1 else "INACTIVO")
# ============================================================================
# CLASE / FUNCIÓN: App
# ============================================================================
# Controlador principal de la aplicación.
#
# Flujo:
#  1. SplashScreen  → sin sidebar
#  2. ConnectionPage → sin sidebar
#  3. Post-conexión  → Sidebar + Vistas
# ============================================================================
class App(ctk.CTk):

    def __init__(self):
        super().__init__()
        ctk.set_appearance_mode("dark")
        ctk.set_default_color_theme("blue")

        self.title(APP_NAME)
        self.geometry(f"{WINDOW_W}x{WINDOW_H}")
        self.resizable(False, False)
        self.configure(fg_color=C["bg"])
        self._center()

        self.backend = PICSerialBackend(data_callback=self._on_packet_received)
        self.grid_rowconfigure(0, weight=1)
        self.grid_columnconfigure(0, weight=1)

        self._container = ctk.CTkFrame(self, fg_color=C["bg"], corner_radius=0)
        self._container.grid(row=0, column=0, sticky="nsew")
        self._container.grid_rowconfigure(0, weight=1)
        self._container.grid_columnconfigure(0, weight=1)

        self._show_splash()

    # ========================================================================
    # FUNCIÓN: _on_packet_received
    # ========================================================================
    # Este método se ejecuta CADA VEZ que el PIC manda un paquete terminado en 0x0A.
    #
    # Flujo:
    #  1. Imprime logs en terminal (diagnóstico).
    #  2. Mapea la estructura de bytes provenientes del dispositivo.
    #  3. Genera un reporte legible.
    #  4. Reparte la información a las pantallas (Logs, Dash, Horarios).
    # ========================================================================
    def _on_packet_received(self, packet):
        print(f"[PIC DATA -> RECOLECTADO]: {packet}")

        # ----------------------------------------------------------------------
        # Fase de mapeo
        # ----------------------------------------------------------------------
        hex_bytes = [f"{byte:02X}" for byte in packet]
        string_hex = " ".join(hex_bytes)

        string_dec = ", ".join(str(byte) for byte in packet)

        if len(packet) == 40:
            timestamp = datetime.now().strftime("%H:%M:%S")
            
            horarios_disp     = packet[0]
            total_horarios    = packet[1]
            pastillas_total   = packet[2]
            cant_compart      = packet[3]
            flag_error_num    = packet[4]
            flag_error        = "⚠️ ERROR ACTIVO" if packet[4] == 1 else "✅ SIN ERROR"

            c1_qty = packet[5]
            c2_qty = packet[6]
            c3_qty = packet[7]
            c4_qty = packet[8]

            reporte = (
                f"╔═══════════════════════════════════════════════════════════════════╗\n"
                f"  📊 REPORTE DE DISPENSACIÓN OPERATIVA ── [{timestamp}]\n"
                f"╚═══════════════════════════════════════════════════════════════════╝\n"
                f" ⚙️ CONFIGURACIÓN GENERAL DEL SISTEMA:\n"
                f"   • Total de dispensaciones realizadas : {horarios_disp}\n"
                f"   • Cantidad de horarios guardados     : {total_horarios} / 6\n"
                f"   • Cantidad total de pastillas        : {pastillas_total}\n"
                f"   • Número de compartimentos config.   : {cant_compart}\n"
                f"   • Estado del Sistema                 : {flag_error}\n\n"
                f" 💊 INVENTARIO ACTUAL DE COMPARTIMENTOS:\n"
                f"   • Compartimento 1 ({NOMBRE_PASTILLA_COMPARTMENTOS[0]}): {c1_qty} pastillas\n"
                f"   • Compartimento 2 ({NOMBRE_PASTILLA_COMPARTMENTOS[1]}): {c2_qty} pastillas\n"
                f"   • Compartimento 3 ({NOMBRE_PASTILLA_COMPARTMENTOS[2]}): {c3_qty} pastillas\n"
                f"   • Compartimento 4 ({NOMBRE_PASTILLA_COMPARTMENTOS[3]}): {c4_qty} pastillas\n\n"
                f" ⏰ DETALLE DE HORARIOS PROGRAMADOS (1 al 6):\n"
            )

            base_idx = 9
            for h in range(6):
                offset = base_idx + (h * 5)
                
                h_hora   = packet[offset]
                h_min    = packet[offset + 1]
                h_ejec   = "Ejecutado" if packet[offset + 2] == 1 else "No Ejecutado"
                h_ocup   = "Ocupado" if packet[offset + 3] == 1 else "Libre"
                h_comp   = packet[offset + 4]

                if h_comp != 0 :
                    reporte += (
                        f"   [ Horario {h+1} ── PROGRAMADO ]\n"
                        f"     > Hora de alarma : {h_hora:02d}:{h_min:02d} hrs\n"
                        f"     > Estado actual  : {h_ejec}\n"
                        f"     > Compartimento  : Comp. {h_comp} ({NOMBRE_PASTILLA_COMPARTMENTOS[h_comp-1] if 1<=h_comp<=4 else 'Indefinido'})\n"
                    )
                else:
                    reporte += f"   [ Horario {h+1} ── Espacio Vacío / {h_ocup.upper()} ]\n"
                reporte += "   " + "─" * 45 + "\n"

            reporte += (
                f"\n 💾 DATA :\n"
                f"   • HEX: {string_hex}\n\n"
                f"   • DEC: ({string_dec})\n"
                f"╔═══════════════════════════════════════════════════════════════════╗\n"
                f"  📊 FIN DEL REPORTE DE DISPENSACIÓN OPERATIVA\n"
                f"╚═══════════════════════════════════════════════════════════════════╝\n"
            )

            # ----------------------------------------------------------------------
            # Fase de inyección a interfaces
            # ----------------------------------------------------------------------
            main_app = None
            if hasattr(self, "_pages"): main_app = self
            elif 'app' in globals(): main_app = globals()['app']

            if main_app and "_pages" in main_app.__dict__:
                
                if "logs" in main_app._pages:
                    logs_page = main_app._pages["logs"]
                    if hasattr(logs_page, "term") and logs_page.term.winfo_exists():
                        logs_page.term.configure(state="normal")
                        logs_page.term.insert("end", reporte)
                        logs_page.term.see("end")
                        logs_page.term.configure(state="disabled")

                if "compartments" in main_app._pages:
                    comps_page = main_app._pages["compartments"]
                    cantidades_actuales = [packet[5], packet[6], packet[7], packet[8]]
                    comps_page.actualizar_compartimentos(cantidades_actuales)

                if "schedule" in main_app._pages:
                    schedule_page = main_app._pages["schedule"]
                    horarios_procesados = []
                    base_idx = 9
                    
                    for h in range(6):
                        offset = base_idx + (h * 5)
                        
                        h_hora   = packet[offset]
                        h_min    = packet[offset + 1]
                        h_ejec   = "Ejecutado" if packet[offset + 2] == 1 else "No Ejecutado"
                        h_comp   = packet[offset + 4]

                        if h_comp != 0:
                            nombre_med = NOMBRE_PASTILLA_COMPARTMENTOS[h_comp - 1] if 1 <= h_comp <= 4 else "Indefinido"
                            txt_comp   = f"Comp. {h_comp}"
                            txt_time   = f"{h_hora:02d}:{h_min:02d} hrs"
                            txt_estado = h_ejec
                        else:
                            nombre_med = "---"
                            txt_comp   = "---"
                            txt_time   = "--:--"
                            txt_estado = "Espacio Vacío"

                        horarios_procesados.append({
                            "id": h + 1,
                            "med": nombre_med,
                            "comp": txt_comp,
                            "time": txt_time,
                            "estado": txt_estado
                        })
                    
                    schedule_page.actualizar_tabla(horarios_procesados)

                if "dashboard" in main_app._pages:
                    dash = main_app._pages["dashboard"]
                    if hasattr(dash, "kpi_pic") and hasattr(dash, "kpi_pic_strip"):
                        if main_app.backend.is_connected:
                            dash.kpi_pic.configure(text="Conectado", text_color=C["green"])
                            dash.kpi_pic_strip.configure(fg_color=C["green"])
                        else:
                            dash.kpi_pic.configure(text="Desconectado", text_color=C["red"])
                            dash.kpi_pic_strip.configure(fg_color=C["red"])

                    if hasattr(dash, "kpi_horarios"):
                        dash.kpi_horarios.configure(text=f"{total_horarios} Guardados")

                    if hasattr(dash, "kpi_vaso"):
                        if flag_error_num == 1:
                            dash.kpi_vaso.configure(text="ERROR", text_color=C["red"])
                        else:
                            dash.kpi_vaso.configure(text="Detectado", text_color=C["green"])

                    capacidad_maxima = CAPACITY_MAX
                    cantidades = [packet[5], packet[6], packet[7], packet[8]]
                    
                    now = datetime.now()
                    minutos_actuales = now.hour * 60 + now.minute

                    proximos_horarios = []
                    horarios_manana = []

                    base_idx = 9
                    for h in range(6):
                        offset = base_idx + (h * 5)
                        
                        h_hora = packet[offset]
                        h_min  = packet[offset + 1]
                        h_comp = packet[offset + 4]

                        if h_comp != 0:
                            minutos_alarma = h_hora * 60 + h_min
                            datos_alarma = {
                                "time_str": f"{h_hora:02d}:{h_min:02d}",
                                "comp_id": h_comp,
                                "minutos": minutos_alarma
                            }
                            
                            if minutos_alarma > minutos_actuales:
                                proximos_horarios.append(datos_alarma)
                            else:
                                horarios_manana.append(datos_alarma)

                    proxima_dosis = None
                    if proximos_horarios:
                        proxima_dosis = min(proximos_horarios, key=lambda x: x["minutos"])
                    elif horarios_manana:
                        proxima_dosis = min(horarios_manana, key=lambda x: x["minutos"])

                    if hasattr(dash, "nxt_med"):
                        if proxima_dosis:
                            c_id = proxima_dosis["comp_id"]
                            nombre_med = NOMBRE_PASTILLA_COMPARTMENTOS[c_id - 1] if 1 <= c_id <= 4 else "Desconocido"
                            
                            dash.nxt_med.configure(text=nombre_med)
                            dash.nxt_comp.configure(text=f"Comp. {c_id}")
                            dash.nxt_time.configure(text=proxima_dosis["time_str"])
                            dash.nxt_qty.configure(text="1 pastilla")
                        else:
                            dash.nxt_med.configure(text="Ninguno")
                            dash.nxt_comp.configure(text="--")
                            dash.nxt_time.configure(text="--:--")
                            dash.nxt_qty.configure(text="--")

                    if hasattr(dash, "nxt_med"):
                        if proxima_dosis:
                            c_id = proxima_dosis["comp_id"]
                            nombre_med = NOMBRE_PASTILLA_COMPARTMENTOS[c_id - 1] if 1 <= c_id <= 4 else "Desconocido"
                            
                            dash.nxt_med.configure(text=nombre_med)
                            dash.nxt_comp.configure(text=f"Comp. {c_id}")
                            dash.nxt_time.configure(text=proxima_dosis["time_str"])
                            dash.nxt_qty.configure(text="1 pastilla")
                        else:
                            dash.nxt_med.configure(text="Ninguno")
                            dash.nxt_comp.configure(text="--")
                            dash.nxt_time.configure(text="--:--")
                            dash.nxt_qty.configure(text="--")

                    if hasattr(dash, "dash_bars") and hasattr(dash, "dash_pcts"):
                        for i in range(4):
                            qty = cantidades[i]
                            porcentaje_flotante = min(1.0, max(0.0, qty / capacidad_maxima))
                            
                            dash.dash_bars[i].set(porcentaje_flotante)
                            dash.dash_pcts[i].configure(text=f"{int(porcentaje_flotante * 100)}%")
        elif len(packet) == 9:
            main_app = self if hasattr(self, "_pages") else globals().get('app')
            if main_app and "diagnostic" in main_app._pages:
                diag_page = main_app._pages["diagnostic"]
                
                # Reconstruimos los valores de 16 bits uniendo el High y el Low
                # Operación: (Byte_Alto multiplicado por 256) + Byte_Bajo
                c1_16bit = (packet[0] << 8) | packet[1]
                c2_16bit = (packet[2] << 8) | packet[3]
                c3_16bit = (packet[4] << 8) | packet[5]
                c4_16bit = (packet[6] << 8) | packet[7]
                
                ir_8bit  = packet[8]
                
                # Enviamos los valores reales a la pantalla
                diag_page.actualizar_sensores(c1_16bit, c2_16bit, c3_16bit, c4_16bit, ir_8bit)
                
    # ========================================================================
    # FUNCIONES DE CONTROL PRINCIPAL Y NAVEGACIÓN
    # ========================================================================
    
    def _center(self):
        self.update_idletasks()
        x = (self.winfo_screenwidth()  - WINDOW_W) // 2
        y = (self.winfo_screenheight() - WINDOW_H) // 2
        self.geometry(f"{WINDOW_W}x{WINDOW_H}+{x}+{y}")

    def _clear_container(self):
        for w in self._container.winfo_children():
            w.destroy()

    def _show_splash(self):
        self._clear_container()
        SplashScreen(self._container,
                     on_continue=self._show_connection).grid(
            row=0, column=0, sticky="nsew")

    def _show_connection(self):
        self._clear_container()
        ConnectionPage(self._container,
                       on_connect=self._launch_workspace,
                       on_back=self._show_splash).grid(
            row=0, column=0, sticky="nsew")

    # ========================================================================
    # FUNCIÓN: _launch_workspace
    # ========================================================================
    # Destruye la vista actual y crea el entorno de trabajo:
    # Sidebar a la izquierda + área de contenido a la derecha.
    # ========================================================================
    def _launch_workspace(self):
        self._clear_container()

        self._container.grid_columnconfigure(0, weight=0)  
        self._container.grid_columnconfigure(1, weight=1)  

        self._sidebar = Sidebar(self._container, on_navigate=self._navigate)
        self._sidebar.grid(row=0, column=0, sticky="nsew")

        ctk.CTkFrame(self._container, width=1, fg_color=C["border"],
                     corner_radius=0).grid(row=0, column=0, sticky="nse")

        self._content = ctk.CTkFrame(self._container, fg_color=C["bg"], corner_radius=0)
        self._content.grid(row=0, column=1, sticky="nsew")
        self._content.grid_rowconfigure(0, weight=1)
        self._content.grid_columnconfigure(0, weight=1)

        self._pages = {
            "home"        : HomePage(self._content),
            "dashboard"   : DashboardPage(self._content),
            "compartments": CompartmentsPage(self._content),
            "schedule"    : SchedulePage(self._content),
            "diagnostic"  : DiagnosticPage(self._content),
            "logs"        : LogsPage(self._content),
        }

        for page in self._pages.values():
            page.grid(row=0, column=0, sticky="nsew")

        self._navigate("dashboard")

    def _navigate(self, page_id: str):
        if page_id in self._pages:
            self._pages[page_id].lift()
            self._sidebar.set_active(page_id)


# ============================================================================
# PUNTO DE ENTRADA
# ============================================================================
if __name__ == "__main__":
    ctk.set_appearance_mode("dark")
    ctk.set_default_color_theme("blue")
    app = App()
    app.mainloop()