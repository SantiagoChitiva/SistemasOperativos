import os
import pandas as pd

def convertir_dat_a_csv(nombre_archivo):
    with open(nombre_archivo, 'r') as archivo:
        lineas = archivo.readlines()
    
    tiempos = []
    for linea in lineas:
        if 'µs' in linea:
            tiempo = linea.split('->')[1].strip().split(' ')[0]
            tiempos.append(int(tiempo))
    
    df = pd.DataFrame(tiempos, columns=['Tiempo (µs)'])
    
    nombre_csv = nombre_archivo.replace('.dat', '.csv')
    
    df.to_csv(nombre_csv, index=False)
    print(f'Archivo convertido: {nombre_csv}')

def convertir_todos_los_dat_a_csv(directorio, palabra_clave):
    for archivo in os.listdir(directorio):
        if archivo.endswith('.dat') and palabra_clave in archivo:
            convertir_dat_a_csv(os.path.join(directorio, archivo))

directorio_actual = '.'
palabra_clave = 'transpuesta'

convertir_todos_los_dat_a_csv(directorio_actual, palabra_clave)
