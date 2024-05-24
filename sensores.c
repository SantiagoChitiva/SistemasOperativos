/*
 * Fecha: 29/04/2024
 * Autor: Santiago Chitiva Contreras, Martin Medina Novoa, Andres David Rueda Sandoval
 * Materia: Sistemas Operativos
 * Tema: Proyecto de Curso
 * Objetivo:
    Implementar el apartado de "sensores" en el proyecto para eso se realiza

      1. Se  verifica que la invocacion del programa sea como esta establecido
      2. Se guardan los parametros de entrada y segun eso se hace
        2.1 Se lee el tipo de sensor indicado con el nombre del archivo
        2.2 Se guarda el tiempo indicado
        2.3 Se crea el pipe con el nombre indicado y con la espera (tiempo) que
se ingreso

-------------------------------------------------------------------------------

    Se implemento/Ya funciona

    1. verificar formato de ejecucion (Sin importar el orde, que esten los
datos solicitados)
    2. leer archivo indicado con el tipo de sensor
    3. agrupar datos segun sensor
    4. pipe para la comunicacion

    Ejecucion:

      1. Ejecutar primero el programa Monitor.c
      2. Este programa se ejecuta de esta manera:./sensores -s 2 -t 3 -f datosPh.txt -p pipe1
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

// Maximo de datos a leer del archivo
#define MAX 1000

// Estrucutra que facilita la lectura de los "sensores"
typedef struct {
  int numDatos;
  char *datos[MAX];
} sensoresData;

// Estructura que facilita manejar la informacion que llega al invocar el
// programa
typedef struct {
  const char *tipo_sensor;
  const char *tiempo;
  const char *archivo;
  const char *pipe_nominal;
} Argumentos;

// int contPh, contTemp;

// Funcion que lee el archivo indicado
sensoresData leerArchivo(const char *fileName) {
  FILE *file;
  sensoresData data;

  char linea[MAX];
  data.numDatos = 0;
  int i = 0;
  file = fopen(fileName, "r");

  // verificar que no este vacio
  if (file == NULL) {
    fputs("Error en el archivo", stderr);
    exit(-1);
  } else { // obtiene los datos del archivo linea por linea
    while (fgets(linea, MAX, file) != NULL && data.numDatos < MAX) {
      data.datos[data.numDatos] = strdup(linea);
      data.numDatos++;
    }
  }

  return data;
}

// Funcion que verifican los datos ingresadps y la forma establecida de
// invocacion del programa
int verificar_argumentos(int argc, const char *argv[], Argumentos *args) {
  // ciclo desde 1 hasra la cantidad de datos ingresado al momento de ejecutar
  // el programa
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-s") == 0) {
      if (i + 1 < argc) {
        args->tipo_sensor = argv[i + 1];
        i++;
      } else {
        return 1;
      }
    } else if (strcmp(argv[i], "-t") == 0) {
      if (i + 1 < argc) {
        args->tiempo = argv[i + 1];
        i++;
      } else {
        return 2;
      }
    } else if (strcmp(argv[i], "-f") == 0) {
      if (i + 1 < argc) {
        args->archivo = argv[i + 1];
        i++;
      } else {
        return 3;
      }
    } else if (strcmp(argv[i], "-p") == 0) {
      if (i + 1 < argc) {
        args->pipe_nominal = argv[i + 1];
        i++;
      } else {
        return 4;
      }
    } else {
      return -1;
    }
  }

  // Verifica que no sean null, es decir los datos esten
  // Falta verificar que los que sea enteros se ingrese dato entero y los
  // strings sean strings
  if (args->tipo_sensor == NULL || args->tiempo == NULL ||
      args->archivo == NULL || args->pipe_nominal == NULL) {
    // printf("faltan argumentos al invocarlo \n");
    return -2;
  }

  return 0;
}

// Funcion principal del programa
int main(int argc, const char *argv[]) {

  // variables
  sensoresData datosRecibidos;
  Argumentos args = {NULL, NULL, NULL, NULL};
  int resultado = verificar_argumentos(argc, argv, &args);

  // Verificar argumentos
  if (resultado == 0) {
    printf("Tipo de sensor: %s\n", args.tipo_sensor);
    printf("Tiempo: %s\n", args.tiempo);
    printf("Archivo: %s\n", args.archivo);
    printf("Pipe nominal: %s\n", args.pipe_nominal);
  } else if (resultado == -1) {
    printf("Error: Opción no reconocida.\n");
  } else if (resultado == -2) {
    printf("Error: Falta uno o más argumentos.\n");
  } else {
    printf("Error: Falta el valor para la opción -%c.\n", (char)resultado);
  }

  // Llenar datos según el nombre del archivo
  if (args.tipo_sensor != NULL && args.archivo != NULL) {
    if (strcmp(args.tipo_sensor, "1") == 0 &&
        strcmp(args.archivo, "datosTemp.txt") == 0) {
      datosRecibidos = leerArchivo(args.archivo);
    } else if (strcmp(args.tipo_sensor, "2") == 0 &&
               strcmp(args.archivo, "datosPh.txt") == 0) {
      datosRecibidos = leerArchivo(args.archivo);
    } else {
      printf("El sensor no existe o el archivo no es válido.\n");
      return -1;
    }
  } else {
    printf("falta el tipo de sensor o el nombre del archivo.\n");
    return -1;
  }

  // pipe
  mkfifo(args.pipe_nominal, 0666);
  int tuberia = open(args.pipe_nominal, O_WRONLY);

  if (tuberia == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < datosRecibidos.numDatos; i++) {
    char *linea = datosRecibidos.datos[i];
    size_t longitud = strlen(linea);

    ssize_t bytesEscritos = write(tuberia, linea, longitud);
    if (bytesEscritos == -1) {
      perror("write");
      close(tuberia);
      exit(EXIT_FAILURE);
    }
  }

  close(tuberia);

  return resultado != 0;
}