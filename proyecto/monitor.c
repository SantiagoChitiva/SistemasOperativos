/*
 * Fecha: 29/04/2024
 * Autor: Santiago Chitiva Contreras, Martin Medina Novoa, Andres David Rueda Sandoval
 * Materia: Sistemas Operativos
 * Tema: Proyecto de Curso
 * Objetivo:
 * Implementar el apartado de "monitor" en el proyecto para eso se realiza
 * 1. Se verifica que la invocacion del programa sea como esta establecido
 * /monitor –b tam_buffer –t file-temp –h file-ph -p pipe-nomina
 * 2. Se tienen que implementar tres hilos a partir de los datos recibidos
 * 2.1 H-recolector: recibe los dos tipos de medidas y los guarda en un
 * buffer dependiendo de si es PH o Temperatura, verifica si el valor recibido
 * es erroneo o correcto (Parametro de claidad) 2.2 H-ph: recibe las medias del
 * buffer de ph y los guarda en un archivo 2.3 H-temperatura: recibe las medias
 * del buffer de temperatura y los guarda en un archivo
 *
 * Ejecucion:
 * 1. Este programa se ejecuta de esta manera: ./monitor -b 1 -t file-temp.txt -h file-ph.txt -p pipe1
 * 2. Ejecutar el programa sensores.c
 */

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define MAX_BUF 1024

// Buffer circular y variables de control
typedef struct {
    float buffer[MAX_BUF];
    int in, out, count, size;
    pthread_mutex_t mutex;
    sem_t sem_full, sem_empty;
} Buffer;

// Buffer para temperatura y pH
Buffer temperatura_buffer, ph_buffer;

// Estructura para manejar los argumentos de entrada
typedef struct {
    int tam_buffer;
    const char *file_temp; // Utilizaremos file_temp para ambos tipos de medidas
    const char *pipe_nominal;
} Argumentos;

// Función para inicializar un buffer
void buffer_init(Buffer *buf, int size) {
    buf->in = buf->out = buf->count = 0;
    buf->size = size;
    pthread_mutex_init(&buf->mutex, NULL);
    sem_init(&buf->sem_full, 0, 0);
    sem_init(&buf->sem_empty, 0, size);
}

// Función para agregar un elemento al buffer
void buffer_add(Buffer *buf, float value) {
    sem_wait(&buf->sem_empty);
    pthread_mutex_lock(&buf->mutex);
    buf->buffer[buf->in] = value;
    buf->in = (buf->in + 1) % buf->size;
    buf->count++;
    pthread_mutex_unlock(&buf->mutex);
    sem_post(&buf->sem_full);
}

// Función para obtener un elemento del buffer
float buffer_get(Buffer *buf) {
    sem_wait(&buf->sem_full);
    pthread_mutex_lock(&buf->mutex);
    float value = buf->buffer[buf->out];
    buf->out = (buf->out + 1) % buf->size;
    buf->count--;
    pthread_mutex_unlock(&buf->mutex);
    sem_post(&buf->sem_empty);
    return value;
}

// Función para verificar los argumentos de entrada
int verificar_argumentos(int argc, const char *argv[], Argumentos *args) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-b") == 0) {
            if (i + 1 < argc) {
                args->tam_buffer = atoi(argv[i + 1]);
                i++;
            } else {
                return 1;
            }
        } else if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                args->file_temp = argv[i + 1];
                i++;
            } else {
                return 2;
            }
        } else if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) {
                args->pipe_nominal = argv[i + 1];
                i++;
            } else {
                return 3;
            }
        } else {
            return -1;
        }
    }

    if (args->tam_buffer <= 0 || args->file_temp == NULL || args->pipe_nominal == NULL) {
        printf("Faltan argumentos o argumentos inválidos al invocarlo\n");
        return -2;
    }

    return 0;
}

// Función para obtener la hora actual en formato de cadena
void obtener_hora_actual(char *buffer, size_t buffer_size) {
    time_t ahora = time(NULL);
    struct tm *tm_info = localtime(&ahora);
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Función del hilo H-recolector
void *recolector(void *arg) {
    Argumentos *args = (Argumentos *)arg;
    const char *pipe_nominal = args->pipe_nominal;

    // Abre el pipe nominal para lectura
    int tuberia = open(pipe_nominal, O_RDONLY);
    if (tuberia == -1) {
        perror("open");
        pthread_exit(NULL);
    }

    char buf[MAX_BUF];
    ssize_t bytesRead;

    while (1) {
        bytesRead = read(tuberia, buf, sizeof(buf) - 1);
        if (bytesRead <= 0) {
            sleep(10); // Espera 10 segundos si no hay sensores conectados
            printf("No hay sensores conectados. Terminando el hilo recolector.\n");
            break;
        }

        buf[bytesRead] = '\0';

        if (strcmp(buf, "FIN") == 0) {
            printf("Mensaje de finalización recibido. Terminando el hilo recolector.\n");
            buffer_add(&ph_buffer, -1); // Señal de terminación
            buffer_add(&temperatura_buffer, -1); // Señal de terminación
            break;
        }

        float valor = atof(buf + 4);
        if (valor < 0) {
            printf("Valor erróneo recibido: %s. Descartando.\n", buf);
            continue;
        }

        if (strstr(buf, "TEMP:") == buf) {
            buffer_add(&temperatura_buffer, valor);
        } else if (strstr(buf, "PH:") == buf) {
            buffer_add(&ph_buffer, valor);
        }

        printf("Mensaje recibido desde el emisor: %s\n", buf);
    }

    close(tuberia);
    printf("Finalización del procesamiento de medidas.\n");
    pthread_exit(NULL);
}

// Función para escribir las mediciones en el archivo
void escribir_medida(const char *file, const char *tipo, float valor) {
    FILE *fp = fopen(file, "a");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    char hora_actual[20];
    obtener_hora_actual(hora_actual, sizeof(hora_actual));

    fprintf(fp, "%s [%s] %.2f\n", hora_actual, tipo, valor);
    fflush(fp);

    fclose(fp);
}

// Función del hilo H-ph
void *ph_hilo(void *arg) {
    Argumentos *args = (Argumentos *)arg;
    const char *file_temp = args->file_temp;

    while (1) {
        float ph_valor = buffer_get(&ph_buffer);
        if (ph_valor == -1) {
            break; // Señal de terminación
        }

        escribir_medida(file_temp, "PH", ph_valor);

        if (ph_valor < 0 || ph_valor > 14) {
            printf("Alerta: Medida de pH fuera de rango: %.2f\n", ph_valor);
        }
    }

    pthread_exit(NULL);
}

// Función del hilo H-temperatura
void *temperatura_hilo(void *arg) {
    Argumentos *args = (Argumentos *)arg;
    const char *file_temp = args->file_temp;

    while (1) {
        float temp_valor = buffer_get(&temperatura_buffer);
        if (temp_valor == -1) {
            break; // Señal de terminación
        }

        escribir_medida(file_temp, "TEMP", temp_valor);

        if (temp_valor < -50 || temp_valor > 150) {
            printf("Alerta: Medida de temperatura fuera de rango: %.2f\n", temp_valor);
        }
    }

    pthread_exit(NULL);
}

// Función principal
int main(int argc, const char *argv[]) {
    // Verificar entradas
    Argumentos args = {0, NULL, NULL};
    int resultado = verificar_argumentos(argc, argv, &args);

    // Verificar argumentos
    if (resultado == 0) {
        printf("Pipe nominal: %s\n", args.pipe_nominal);
    } else if (resultado == -1) {
        printf("Error: Opción no reconocida.\n");
        return -1;
    } else if (resultado == -2) {
        printf("Error: Falta uno o más argumentos o argumentos inválidos.\n");
        return -1;
    } else {
        printf("Error: Falta el valor para la opción -%c.\n", (char)resultado);
        return -1;
    }

    // Inicializar buffers
    buffer_init(&temperatura_buffer, args.tam_buffer);
    buffer_init(&ph_buffer, args.tam_buffer);

    // Crear los hilos
    pthread_t recolector_thread, ph_thread, temp_thread;
    pthread_create(&recolector_thread, NULL, recolector, &args);
    pthread_create(&ph_thread, NULL, ph_hilo, &args);
    pthread_create(&temp_thread, NULL, temperatura_hilo, &args);

    // Esperar a que los hilos terminen
    pthread_join(recolector_thread, NULL);
    pthread_join(ph_thread, NULL);
    pthread_join(temp_thread, NULL);

    return 0;
}
