#!/usr/bin/env python3
"""
DM2 — Práctica 2: Señales Físicas
Visualizador en tiempo real de muestras ADC por puerto serie.

Uso:
    python3 plot_serial.py [puerto] [baudrate]

Ejemplos:
    python3 plot_serial.py /dev/ttyUSB0 115200   (Linux)
    python3 plot_serial.py /dev/tty.usbserial-* 115200   (Mac)
    python3 plot_serial.py COM3 115200   (Windows)

Espera datos en el formato que genera stream_adc():
    un entero por línea, p.ej. "2048\r\n"

Requiere:
    pip install pyserial matplotlib
"""

import sys
import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# ── Configuración ─────────────────────────────────────────────────────────
PORT     = sys.argv[1] if len(sys.argv) > 1 else "/dev/ttyUSB0"
BAUDRATE = int(sys.argv[2]) if len(sys.argv) > 2 else 115200
WINDOW   = 200    # muestras visibles en la ventana deslizante
ADC_MAX  = 4095   # 12-bit

# ── Estado ────────────────────────────────────────────────────────────────
data = deque([0] * WINDOW, maxlen=WINDOW)

# ── Conexión serie ────────────────────────────────────────────────────────
try:
    ser = serial.Serial(PORT, BAUDRATE, timeout=1)
    print(f"Conectado a {PORT} @ {BAUDRATE} baud")
except serial.SerialException as e:
    print(f"Error abriendo {PORT}: {e}")
    print("Puertos disponibles:")
    import serial.tools.list_ports
    for p in serial.tools.list_ports.comports():
        print(f"  {p.device}  —  {p.description}")
    sys.exit(1)

# ── Plot ──────────────────────────────────────────────────────────────────
fig, (ax_signal, ax_hist) = plt.subplots(2, 1, figsize=(10, 6))
fig.suptitle("DM2 — ADC Stream en vivo", fontsize=12)

# Señal temporal
line, = ax_signal.plot([], [], color="#00f0ff", linewidth=1)
ax_signal.set_xlim(0, WINDOW)
ax_signal.set_ylim(-50, ADC_MAX + 50)
ax_signal.set_ylabel("ADC (LSB)")
ax_signal.set_xlabel("Muestras")
ax_signal.grid(True, alpha=0.3)

# Estadísticas en el título
stats_text = ax_signal.text(
    0.01, 0.95, "", transform=ax_signal.transAxes,
    fontsize=9, verticalalignment="top",
    bbox=dict(boxstyle="round", facecolor="#111", alpha=0.7),
    color="#ffffff"
)

# Histograma
ax_hist.set_xlabel("ADC (LSB)")
ax_hist.set_ylabel("Frecuencia")
ax_hist.set_title("Distribución (últimas muestras)")
ax_hist.grid(True, alpha=0.3)

def update(_frame):
    # Leer todas las líneas disponibles
    while ser.in_waiting:
        try:
            raw = ser.readline().decode("ascii", errors="ignore").strip()
            if raw.isdigit():
                data.append(int(raw))
        except Exception:
            pass

    d = list(data)
    line.set_data(range(len(d)), d)

    # Estadísticas
    if len(d) > 1:
        mean = sum(d) / len(d)
        var  = sum((x - mean) ** 2 for x in d) / len(d)
        mn   = min(d)
        mx   = max(d)
        stats_text.set_text(
            f"media={mean:.1f}  var={var:.1f}  min={mn}  max={mx}  rango={mx-mn}"
        )

    # Histograma
    ax_hist.cla()
    ax_hist.hist(d, bins=40, color="#ff007f", alpha=0.7, edgecolor="none")
    ax_hist.set_xlabel("ADC (LSB)")
    ax_hist.set_ylabel("Frecuencia")
    ax_hist.grid(True, alpha=0.3)
    fig.canvas.flush_events()

    return line, stats_text

ani = animation.FuncAnimation(fig, update, interval=50, blit=False)
plt.tight_layout()
plt.show()

ser.close()
