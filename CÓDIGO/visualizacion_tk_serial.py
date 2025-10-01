# visualizacion_tk_serial.py
import tkinter as tk
from tkinter import ttk
import serial
import threading
import time
from collections import deque
import matplotlib
matplotlib.use("TkAgg")
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

# Configuración (ajusta puerto según tu OS)
SERIAL_PORT = "COM3"      # Windows ej. "COM3"; Linux ej. "/dev/ttyUSB0"
BAUDRATE = 115200

# Buffer para plot (últimos N valores)
MAX_POINTS = 60
datos = deque(maxlen=MAX_POINTS)
timestamps = deque(maxlen=MAX_POINTS)

# Conectar serial en hilo
ser = None
stop_thread = False

def lector_serial():
    global ser, stop_thread
    try:
        ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
    except Exception as e:
        print("Error abriendo puerto serial:", e)
        return
    while not stop_thread:
        try:
            linea = ser.readline().decode('utf-8').strip()
            if not linea:
                continue
            # Esperamos formato: TS,RAW,HUM
            partes = linea.split(',')
            if len(partes) >= 3:
                try:
                    ts = int(partes[0])
                    raw = int(partes[1])
                    hum = int(partes[2])
                except:
                    continue
                timestamps.append(ts/1000.0)  # segundos
                datos.append(hum)
            # si quieres, imprimir linea completa
            # print("Serial:", linea)
        except Exception as e:
            print("Error lectura serial:", e)
            time.sleep(0.5)
    if ser and ser.is_open:
        ser.close()

# GUI
class App:
    def __init__(self, root):
        self.root = root
        root.title("Monitor de Humedad - Tiempo Real")
        root.geometry("500x360")

        self.label = ttk.Label(root, text="Humedad: -- %", font=("Arial", 16))
        self.label.pack(pady=10)

        # Boton de reconectar
        self.btn_frame = ttk.Frame(root)
        self.btn_frame.pack()
        self.btn_start = ttk.Button(self.btn_frame, text="Iniciar lectura", command=self.start)
        self.btn_start.grid(row=0, column=0, padx=5)
        self.btn_stop = ttk.Button(self.btn_frame, text="Detener lectura", command=self.stop, state="disabled")
        self.btn_stop.grid(row=0, column=1, padx=5)

        # Plot matplotlib embebido
        self.fig = Figure(figsize=(5,2.5))
        self.ax = self.fig.add_subplot(111)
        self.ax.set_ylim(0, 100)
        self.ax.set_ylabel("% Humedad")
        self.ax.set_xlabel("Tiempo (s)")
        self.line, = self.ax.plot([], [], lw=1)

        self.canvas = FigureCanvasTkAgg(self.fig, master=root)
        self.canvas.get_tk_widget().pack(fill="both", expand=True, padx=10, pady=10)

        # Actualizador GUI
        self.update_gui()

    def start(self):
        global lector_thread, stop_thread
        stop_thread = False
        lector_thread = threading.Thread(target=lector_serial, daemon=True)
        lector_thread.start()
        self.btn_start.config(state="disabled")
        self.btn_stop.config(state="normal")

    def stop(self):
        global stop_thread
        stop_thread = True
        self.btn_start.config(state="normal")
        self.btn_stop.config(state="disabled")

    def update_gui(self):
        # Actualiza etiqueta con último valor
        if datos:
            self.label.config(text=f"Humedad: {datos[-1]} %")
            # actualizar plot
            x = list(range(-len(datos)+1, 1))  # eje relativo (últimos N)
            y = list(datos)
            self.line.set_data(x, y)
            self.ax.set_xlim(min(x)-1, 0+1)
            self.canvas.draw_idle()
        self.root.after(500, self.update_gui)  # actualizar cada 500 ms

if __name__ == "__main__":
    root = tk.Tk()
    app = App(root)
    root.mainloop()
