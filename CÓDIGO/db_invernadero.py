# db_invernadero.py
# Sprint 2 - Victor
# Lógica de base de datos + pruebas de consultas

import sqlite3
from datetime import datetime

# ---------- CONFIGURACIÓN ----------
DB_NAME = "invernadero.db"

# ---------- CREAR TABLA ----------
def crear_tabla():
    conn = sqlite3.connect(DB_NAME)
    cur = conn.cursor()
    cur.execute("""
        CREATE TABLE IF NOT EXISTS lecturas (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            fecha TEXT,
            hora TEXT,
            humedad INTEGER,
            temperatura REAL
        )
    """)
    conn.commit()
    conn.close()

# ---------- REGISTRAR LECTURA ----------
def registrar_lectura(humedad, temperatura):
    conn = sqlite3.connect(DB_NAME)
    cur = conn.cursor()
    ahora = datetime.now()
    fecha = ahora.strftime("%Y-%m-%d")
    hora = ahora.strftime("%H:%M:%S")
    cur.execute("INSERT INTO lecturas (fecha,hora,humedad,temperatura) VALUES (?,?,?,?)",
                (fecha, hora, humedad, temperatura))
    conn.commit()
    conn.close()

# ---------- CONSULTAS ----------
def mostrar_ultimas(n=5):
    conn = sqlite3.connect(DB_NAME)
    cur = conn.cursor()
    cur.execute("SELECT * FROM lecturas ORDER BY id DESC LIMIT ?", (n,))
    filas = cur.fetchall()
    conn.close()
    return filas

def promedio_humedad():
    conn = sqlite3.connect(DB_NAME)
    cur = conn.cursor()
    cur.execute("SELECT AVG(humedad) FROM lecturas")
    promedio = cur.fetchone()[0]
    conn.close()
    return promedio

# ---------- PRUEBAS ----------
if __name__ == "__main__":
    crear_tabla()

    # Prueba: registrar lecturas falsas (luego se reemplazan con datos del ESP32)
    registrar_lectura(55, 22.5)
    registrar_lectura(60, 23.1)
    registrar_lectura(48, 21.9)

    print("Últimas lecturas:")
    for fila in mostrar_ultimas(3):
        print(fila)

    print("Promedio de humedad:", promedio_humedad())