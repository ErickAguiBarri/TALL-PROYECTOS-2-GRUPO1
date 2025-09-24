import tkinter as tk
from tkinter import ttk, messagebox
import random

class InvernaderoApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Invernadero Automatizado")
        self.root.geometry("500x400")
        self.root.configure(bg="#eaf7ea")

        # Variables simuladas (normalmente vendrían de Arduino)
        self.temperatura = tk.DoubleVar(value=22.5)
        self.humedad = tk.DoubleVar(value=60.0)
        self.riego_activo = tk.BooleanVar(value=False)

        # Crear pestañas (Notebook)
        self.notebook = ttk.Notebook(self.root)
        self.notebook.pack(expand=True, fill="both")

        # Pestañas
        self.crear_pantalla_inicio()
        self.crear_pantalla_control()
        self.crear_pantalla_configuracion()

        # Simular actualización de sensores
        self.actualizar_sensores()

    def crear_pantalla_inicio(self):
        """Pantalla principal (p1)"""
        frame = tk.Frame(self.notebook, bg="#eaf7ea")
        self.notebook.add(frame, text="Inicio")

        tk.Label(frame, text="🌱 Estado del Invernadero", font=("Arial", 16, "bold"), bg="#eaf7ea").pack(pady=10)

        self.label_temp = tk.Label(frame, text=f"🌡️ Temperatura: {self.temperatura.get()} °C", font=("Arial", 14), bg="#eaf7ea")
        self.label_temp.pack(pady=5)

        self.label_hum = tk.Label(frame, text=f"💧 Humedad: {self.humedad.get()} %", font=("Arial", 14), bg="#eaf7ea")
        self.label_hum.pack(pady=5)

        self.label_riego = tk.Label(frame, text="🚰 Riego: Inactivo", font=("Arial", 14), bg="#eaf7ea", fg="red")
        self.label_riego.pack(pady=5)

    def crear_pantalla_control(self):
        """Pantalla de control (p2)"""
        frame = tk.Frame(self.notebook, bg="#f0f7ff")
        self.notebook.add(frame, text="Control")

        tk.Label(frame, text="⚙️ Control Manual", font=("Arial", 16, "bold"), bg="#f0f7ff").pack(pady=10)

        boton_riego = tk.Button(frame, text="Encender/Apagar Riego", command=self.toggle_riego, bg="#cce5ff", font=("Arial", 12))
        boton_riego.pack(pady=10)

        boton_ventilacion = tk.Button(frame, text="Encender Ventilación", command=lambda: messagebox.showinfo("Ventilación", "Ventilación activada ✅"), bg="#cce5ff", font=("Arial", 12))
        boton_ventilacion.pack(pady=10)

    def crear_pantalla_configuracion(self):
        """Pantalla de configuración (p3)"""
        frame = tk.Frame(self.notebook, bg="#fff7e6")
        self.notebook.add(frame, text="Configuración")

        tk.Label(frame, text="🛠️ Configuración de Riego Automático", font=("Arial", 16, "bold"), bg="#fff7e6").pack(pady=10)

        tk.Label(frame, text="Intervalo de riego (minutos):", bg="#fff7e6").pack(pady=5)
        self.intervalo_riego = tk.IntVar(value=30)
        tk.Entry(frame, textvariable=self.intervalo_riego, width=10).pack(pady=5)

        tk.Button(frame, text="Guardar Configuración", command=self.guardar_config, bg="#ffd480").pack(pady=10)

    def toggle_riego(self):
        self.riego_activo.set(not self.riego_activo.get())
        estado = "Activo ✅" if self.riego_activo.get() else "Inactivo ❌"
        color = "green" if self.riego_activo.get() else "red"
        self.label_riego.config(text=f"🚰 Riego: {estado}", fg=color)

    def guardar_config(self):
        mins = self.intervalo_riego.get()
        messagebox.showinfo("Configuración", f"El riego automático se programó cada {mins} minutos ✅")

    def actualizar_sensores(self):
        """Simulación de cambios en los sensores"""
        self.temperatura.set(round(random.uniform(18, 28), 1))
        self.humedad.set(round(random.uniform(40, 80), 1))

        self.label_temp.config(text=f"🌡️ Temperatura: {self.temperatura.get()} °C")
        self.label_hum.config(text=f"💧 Humedad: {self.humedad.get()} %")

        # Actualiza cada 3 segundos
        self.root.after(3000, self.actualizar_sensores)


# Ejecutar la app
if __name__ == "__main__":
    root = tk.Tk()
    app = InvernaderoApp(root)
    root.mainloop()

