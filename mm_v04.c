/**********************************************************
 * Fecha: 11/4/2024
 * Autor: Santiago Chitiva Contreras
 * Materia: Sistemas operativos
 * Tema: Concurrencia
 * Objetivo: Hacer una metodología para la implementación de la multiplicación de matrices.
 * La idea principal, es construir paso a paso la implementación para hacer uso de la biblioteca PTHREAD. Se implementa el Algoritmo Clásico de multiplicación de matrices, para matrices cuadradas, es decir, la dimensión de filas es igual a la de columnas.
 A continuación de describen los pasos:
  - Reserva de memoria
  - Creación de punteros para las matrices de tipo double
  - Asignación de memoria
  - Ingreso de argumentos de entrada (dimensión matriz, número de hilos)
  - Validación argumentos de entrada
  - Inicializar las matrices
  - Imprimir las matrices
  - Función para inicializar las matrices
  - Función para imprimir las matrices
  - Algoritmo clásico de multiplicación de matrices
  - Se verifica el resultado
  - Función para multipliación de matrices
  - Declaración de vector de hilos
  - Creación de hilos según tamaño de vector de hilos
  - Crear estructura de datos que encapsule los argumentos de entrada de la función MM
  
* Implementación de paralelismo: resolución de la multiplicación de matrices
  - Se crea el vector de hilos
  - Se tiene pendiente la exlusión de los hilos
  - Se pasa a globales las matrices
  - Encapsular los datos para envirlos a la función MxM
  - Se desencapsulan los datos dentro de la función MxM (descomprimen)
  
* Instrucciones y consideraciones de uso:
  - La multiplicación de matrices se realiza en la función multiMatrices, donde cada hilo calcula una parte de la matriz resultado.
  - Las matrices se inicializan en la función initMatrices, donde se asignan valores predefinidos a cada elemento de las matrices.
  - La función imprMatrices se utiliza para imprimir por pantalla las matrices, facilitando la verificación de los resultados.
  - Para ejecutar el programa, se debe proporcionar la dimensión de las matrices y el número de hilos como argumentos.
  - Se debe usar la librería PTHREAD
 **********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#define RESERVA (1024*128*64*8)

struct datosMM{
  int N;          //Dimensión matrices (NxN)
  int H;          //Número de hilos
  int idH;        //Identidad de hilos
};

//Se declaran las variables (apuntadores a las matrices) globales
double *mA, *mB, *mC; 

//Reserva de memoria para guardar las matrices
static double MEM_CHUNK[RESERVA]; 

//Función que multiplica las matrices
void *multiMatrices(void *argMM){
  struct datosMM *intValor = (struct datosMM *) argMM;
  int n = intValor->N; //Dimensión de las matrices
  int h = intValor->H; //Número de hilos
  int idH = intValor->idH; //Identificador del hilo
  
  //Cálculo de las partes de la matriz de las que se encargará cada hilo
  int ini = idH * n / h;
  int fin = (idH + 1) * n / h;

  //Mensaje sobre el trabajo de cada hilo
  printf("Hilos = %d; IdHilo = %d; ini = %d; fin = %d\n", h, idH, ini, fin);

  //Ciclo para multiplicar las matrices con el Algoritmo Clásico
  for(int i=ini; i<fin; i++){
    for(int j=0; j<n; j++){
      double sumaTemp, *pA, *pB;
      sumaTemp = 0.0;
      pA = mA + i*n;
      pB = mB + j;
      for(int k=0; k<n; k++, pA++, pB+=n){
        sumaTemp += *pA * *pB;
      }
      mC[j+i*n] = sumaTemp;
    }
  }
  pthread_exit(NULL);

}

//Función que inicializa las matrices con valores predeterminados
void initMatrices(int n){
  for(int i=0; i<n*n; i++){
    mA[i] = i*1.1;
    mB[i] = i*2.2;
    mC[i] = i;
  }
}

//Función que imprime las matrices
void imprMatrices(int n, double *matriz){
  if(n < 9){
    for(int i=0; i<n*n; i++){
      if(i%n == 0)
        printf("\n");
      printf(" %f ", matriz[i]);
    }
    printf("\n###############################################################\n");
  } else{
    printf("\n##############################################################\n");
  }
}

int main(int argc, char *argv[]) {
    //Verificación de los argumentos
    if(argc <= 2){
    printf("Numero argumentos incorrectos\n");
    printf("\n\t $ejecutable.exe DIM NumHilos\n");
    return -1;
  }

  //Lectura de la dimensión de matrices y número de hilos
  int N = (int) atof(argv[1]); 
  int H = (int) atof(argv[2]); 

  //Verificación de que los valores sean válidos
  if(N <= 0 || H <=0){
    printf("\nValores deben ser mayor que cero");
    return -1;
  }

  //Se declaran los hilos
  pthread_t hilos[H]; 

  //Se reserva memoria para las matrices
  mA = MEM_CHUNK; 
  mB = mA + N*N; 
  mC = mB + N*N; 

  //Se inicializan las matrices
  initMatrices(N);

  //Impresión de las matrices "originales"
  imprMatrices(N, mA); 
  imprMatrices(N, mB); 

  //Creación de los hilos mediante un ciclo
  for(int h=0; h<H; h++){
    struct datosMM *valoresMM = (struct datosMM*)malloc(sizeof(struct datosMM));
    valoresMM->N = N;
    valoresMM->H = H;
    valoresMM->idH = h;
    pthread_create(&hilos[h], NULL, multiMatrices, valoresMM);
  }

  //Ciclo para esperar a que los hilos terminen su ejecución
  for(int h=0; h<H; h++){
    pthread_join(hilos[h], NULL);
  }

  //Impresión de la matriz resultado
  imprMatrices(N, mC); 

  printf("\n\tFin del programa.....!\n");

  pthread_exit(NULL);
}


