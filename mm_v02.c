/**********************************************************
 * Fecha: 4/4/2024
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
 **********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <pthread.h>

#define RESERVA (1024*128*64*8)

// Estructura que encapsula los datos de la función 
struct datosMM{
  int N; //Num. de filas y columnas (dimensión) de las matrices
  int H; //Num. de hilos
  double *mA; //Apuntador a la matriz A
  double *mB; //Apuntador a la matriz B
  double *mC; //Apuntador a la matriz resultado C
};

//Se reserva memoria para almacenar las matrices
static double MEM_CHUNK[RESERVA];

//Función que inicializa los valores de las matrices
void initMatrices(int n, double *m1, double *m2, double *m3){
  for(int i=0; i<n*n; i++){
    m1[i] = i*1.1;
    m2[i] = i*2.2;
    m3[i] = i;
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
    printf("\n#####################################\n");
  } else{
    printf("\n#####################################\n");
  }
}

//Función que multiplica las matrices usando el Algoritmo Clásico
//void *multiMatrices(void *argMM)
void *multiMatrices(int n, double *m1, double *m2, double *m3){
  for(int i=0; i<n; i++){
    for(int j=0; j<n; j++){
      double sumaTemp, *pA, *pB;
      sumaTemp = 0.0;
      pA = m1 + i*n;
      pB = m2 + j;
      for(int k=0; k<n; k++, pA++, pB+=n){
        sumaTemp += *pA * *pB;
      }
      m3[j+i*n] = sumaTemp;
    }
  }
}

int main(int argc, char *argv[]) {
    if(argc <= 2){
    printf("Numero argumentos incorrectos\n");
    printf("\n\t $ejecutable.exe DIM NumHilos\n");
    return -1;
  }
  
  int N = (int) atof(argv[1]); //Ingreso de la dimensión de las matrices
  int H = (int) atof(argv[2]); //Ingreso del número de hilos
  //Validación de los argumentos de entrada
  if(N <= 0 || H <=0){
    printf("\nValores deben ser mayor que cero");
    return -1;
  }

  pthread_t hilos[H]; //Arreglo de hilos
  
  double *mA, *mB, *mC;
  mA = MEM_CHUNK; //Asiganción de memoria a la matriz A
  mB = mA + N*N; //Asiganción de memoria a la matriz B
  mC = mB + N*N; //Asiganción de memoria a la matriz C
  
  initMatrices(N, mA, mB, mC); // Función de inicialización de las matrices llamada en el main
  
  imprMatrices(N, mA); // Impresión de la matriz A llamada en el main
  imprMatrices(N, mB); // Impresión de la matriz B llamada en el main

  //Creación de hilos
  for(int h=0; h<H; h++){
    pthread_create(&hilos[h], NULL, multiMatrices, &h);
  }

  multiMatrices(N, mA, mB, mC); // Función de multiplicación de matrices llamada en el main

  printf("\n*************************************\n");
  
  imprMatrices(N, mC); //Impresión de la matriz resultado
  
  printf("\n\tFin del programa.....!\n");
  
  return 0;
}