/*----------
* Proyecto 3 - Predicciones CUDA
* Andrea Estefania Elias Cobar 17048
* Kevin Sebastian Macario 17369
* David Uriel Soto Alvarez 17551
* ----------
* Universidad del Valle
* Programación de Microprocesadores
* Semestre 4, 2018
* ----------
*/

//Se incluyen las librerias necesarias
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <fstream>
#include <cuda_runtime.h>

//Se definen los valores fijos a utilizar en el programa
#define H 288         //Cada bloque manejara 100 datos correspondientes a 5 minutos de mediciones en intervalos de 3 segundos
#define B 2           //Se trabajaran 2 bloques, uno para cada dia
#define VUELTAS 28800 //Cantidad de datos por arreglo
#define N 30          //Varible utilizada en pruebas

using namespace std;

__global__
void inversion(float *x, float *y)
{
  int i = blockIdx.x*blockDim.x + threadIdx.x;
  if (i < N) y[i] = x[N-1-i];
}

__global__
void raices(float *x, float *y)
{
  int i = blockIdx.x*blockDim.x + threadIdx.x;
  if (i < N) y[i] = sqrt (x[i]);
}

__global__
void potencia3(float *x, float *y)
{
  int i = blockIdx.x*blockDim.x + threadIdx.x;
  if (i < N) y[i] = pow ((double)x[i],3.0);
}

//Subrutina que calcula cual fue la media de los datos mandados por medio de un vector
__global__
void media(float* arreglo)
{
    float sumatoria = 0;
    float med = 0; //54
    for(int i=0;i<VUELTAS;i++){
  	  sumatoria = sumatoria + arreglo[i];	  
    }
    med = sumatoria/(float) VUELTAS;
    sumatoria = med; 
}

//Subrutina que calcula cual fue la mayor medicion en el dia con hora a la que fue medida
__global__
void mayor(float* arreglo){
    float may=arreglo[0];
    for(int i=0;i<VUELTAS;i++)
    { if(arreglo[i]>may){
      may=arreglo[i];}             
    }
}

//Subrutina que calcula cual fue la menor medicion en el dia con hora a la que fue medida
__global__
void menor(float* arreglo){
    float men=arreglo[0];
    for(int i=0;i<VUELTAS;i++)
    { if(arreglo[i]<men){
      men=arreglo[i];}
       
    } 
}

//Subrutina que calcula la prediccion de datos para un dia siguiente a traves de la regresion lineal de un tipo de medicion hecha por cada 5 minutos en intervalos de 3 segundos
__global__
void prediccion(float* arreglo, float* salida){
        int i = blockIdx.x*blockDim.x + threadIdx.x;
        int q = 0;
	float k = 100.0;
	float m = 0;
	float sumatoria = 0;
	float sumasDif = 0;
	float potencia = 0;
	float pendiente = 0;
	//float nueva = 0;
        q = i*100;
	for(int j = q; j<q+100; j++){
		sumatoria = sumatoria + arreglo[j];
	}
	sumatoria = sumatoria/k;
	for(int j = q; j<q+100; j++){
		sumasDif = arreglo[j] - sumatoria;
	}
	potencia = (float)pow((double)sumasDif,2.00);
	pendiente = potencia/k;

	for(int j = q; j<q+100; j++){
		salida[j] = sumatoria + pendiente*m;
		m = m + 1;
	}

}

//Inicio del programa
int main(void)
{
  // declaraciones de componentes CUDA, Streams y memoria
  cudaStream_t stream1, stream2, stream3, stream4, stream5, stream6;
  cudaStreamCreate(&stream1);
  cudaStreamCreate(&stream2);
  cudaStreamCreate(&stream3);
  cudaStreamCreate(&stream4);
  cudaStreamCreate(&stream5);
  cudaStreamCreate(&stream6);

  //Se abren los archivos y se limpian
  ofstream ArchivoPrediccion("181113_estCU.csv");
  ArchivoPrediccion.close();
  ofstream ArchivoPrediccion2("181114_estCU.csv");
  ArchivoPrediccion2.close();

  //Se crean los vectores que guardaran los string de horas de los archivos .csv
  string horas[VUELTAS];
  string horas2[VUELTAS];

  //Se inician las variables que guardaran los tiempos de ejecucion de cada kernel
  float milliseconds1 = 0;
  float milliseconds2 = 0;
  float milliseconds3 = 0;
  float milliseconds4 = 0;
  float milliseconds5 = 0;
  float milliseconds6 = 0;

  //Se crean las variables de vectores que llevaran datos y compiaran entre el host y el device
  float *vectorTemperatura1, *vectorHumedad1, *vectorPresion1, *res_stream1, *res_stream2, *res_stream3;
  float *vectorTemperatura2, *vectorHumedad2, *vectorPresion2, *res_stream4, *res_stream5, *res_stream6;
  float *dev_res1, *dev_res2, *dev_res3;
  float *dev_res4, *dev_res5, *dev_res6;
  // reserva en el host

  // reserva en el device
  cudaMalloc( (void**)&dev_res1, VUELTAS*sizeof(float));
  cudaMalloc( (void**)&dev_res2, VUELTAS*sizeof(float));
  cudaMalloc( (void**)&dev_res3, VUELTAS*sizeof(float));
  cudaMalloc( (void**)&dev_res4, VUELTAS*sizeof(float));
  cudaMalloc( (void**)&dev_res5, VUELTAS*sizeof(float));
  cudaMalloc( (void**)&dev_res6, VUELTAS*sizeof(float));

  //Asignacion de memoria al host
  cudaHostAlloc((void**)&vectorTemperatura1,VUELTAS*sizeof(float),cudaHostAllocDefault);
  cudaHostAlloc((void**)&vectorHumedad1,VUELTAS*sizeof(float),cudaHostAllocDefault);
  cudaHostAlloc((void**)&vectorPresion1,VUELTAS*sizeof(float),cudaHostAllocDefault);
  cudaHostAlloc((void**)&vectorTemperatura2,VUELTAS*sizeof(float),cudaHostAllocDefault);
  cudaHostAlloc((void**)&vectorHumedad2,VUELTAS*sizeof(float),cudaHostAllocDefault);
  cudaHostAlloc((void**)&vectorPresion2,VUELTAS*sizeof(float),cudaHostAllocDefault);

  cudaHostAlloc((void**)&res_stream1,VUELTAS*sizeof(float),cudaHostAllocDefault);
  cudaHostAlloc((void**)&res_stream2,VUELTAS*sizeof(float),cudaHostAllocDefault);
  cudaHostAlloc((void**)&res_stream3,VUELTAS*sizeof(float),cudaHostAllocDefault);
  cudaHostAlloc((void**)&res_stream4,VUELTAS*sizeof(float),cudaHostAllocDefault);
  cudaHostAlloc((void**)&res_stream5,VUELTAS*sizeof(float),cudaHostAllocDefault);
  cudaHostAlloc((void**)&res_stream6,VUELTAS*sizeof(float),cudaHostAllocDefault);

  // se crean los eventos
  cudaEvent_t start, stop;
  cudaEventCreate(&start);
  cudaEventCreate(&stop);

/////////////////////////////////////////////////////////////////////////////////////////////////

  // Inicializacion de datos por lectura de archivos .csv
  // Se leen los datos del dia 1
  ifstream datos("181113.csv");
  string linea;
  int contadorPosicion = 0;

  // Se obtienen los datos separados de cada linea guardada
  while(getline(datos,linea)){
       string delimiter = ";";
       size_t pos = 0;
       string token;
       int cont = 0;
       while ((pos = linea.find(delimiter)) != std::string::npos) {
          token = linea.substr(0, pos);
          linea.erase(0, pos + delimiter.length());
          if(cont == 0){
	     horas[contadorPosicion] = token;
          }
	  if(cont == 1){
	     vectorTemperatura1[contadorPosicion] = (float)(::atof(token.c_str()));
          }
	  if(cont == 2){
	     vectorHumedad1[contadorPosicion] = (float)(::atof(token.c_str()));
          }
	  if(cont == 3){
	     vectorPresion1[contadorPosicion] =  (float)(::atof(token.c_str()));
          }
	  cont = cont + 1;

       }
       contadorPosicion = contadorPosicion + 1;
  }


////////////////////////////////////////////////////////////////////////////////

  //Se ejecutan 3 kernels cada uno en un stream diferente y haciendolo en 288 bloques cada uno, de manera aplicar regresion lineal cada 100 datos equivalente a 5 minutos de mediciones para el dia 1
  for(int i=0;i < H;i++){
  // copia de datos hacia el device
  cudaMemcpyAsync(dev_res1, vectorTemperatura1, VUELTAS*sizeof(float), cudaMemcpyHostToDevice,stream1);

  //Se hace la medicion del tiempo atraves de events
  cudaEventRecord(start);
  prediccion<<<1, H>>>(vectorTemperatura1, dev_res1);
  cudaEventRecord(stop);

  cudaMemcpyAsync(res_stream1, dev_res1, VUELTAS*sizeof(float), cudaMemcpyDeviceToHost,stream1);

  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&milliseconds1, start, stop);

/////////////////////////////////////////////////////////////////////////////

  cudaMemcpyAsync(dev_res2, vectorHumedad1, VUELTAS*sizeof(float), cudaMemcpyHostToDevice,stream2);

  //Se hace la medicion del tiempo atraves de events
  cudaEventRecord(start);
  prediccion<<<1, H>>>(vectorHumedad1, dev_res2);
  cudaEventRecord(stop);

  cudaMemcpyAsync(res_stream2, dev_res2, VUELTAS*sizeof(float), cudaMemcpyDeviceToHost, stream2);

  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&milliseconds2, start, stop);

////////////////////////////////////////////////////////////////////////////////

  cudaMemcpyAsync(dev_res3, vectorPresion1, VUELTAS*sizeof(float), cudaMemcpyHostToDevice,stream3);

  //Se hace la medicion del tiempo atraves de events
  cudaEventRecord(start);
  prediccion<<<1, H>>>(vectorPresion1, dev_res3);
  cudaEventRecord(stop);

  cudaMemcpyAsync(res_stream3, dev_res3, VUELTAS*sizeof(float), cudaMemcpyDeviceToHost,stream3);

  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&milliseconds3, start, stop);

  }

///////////////////////////////////////////////////////////////////////////////

  //Se sincronizan los streams
  cudaStreamSynchronize(stream1); // wait for stream1 to finish
  cudaStreamSynchronize(stream2); // wait for stream2 to finish
  cudaStreamSynchronize(stream3); // wait for stream3 to finish

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // Se leen los datos del dia 2
  ifstream datos2("181114.csv");
  contadorPosicion = 0;

  // Se obtienen los datos separados de cada linea guardada
  while(getline(datos2,linea)){
       string delimiter = ";";
       size_t pos = 0;
       string token;
       int cont = 0;
       while ((pos = linea.find(delimiter)) != std::string::npos) {
          token = linea.substr(0, pos);
          linea.erase(0, pos + delimiter.length());
          if(cont == 0){
	     horas2[contadorPosicion] = token;
          }
	  if(cont == 1){
	     vectorTemperatura2[contadorPosicion] = (float)(::atof(token.c_str()));
          }
	  if(cont == 2){
	     vectorHumedad2[contadorPosicion] = (float)(::atof(token.c_str()));
          }
	  if(cont == 3){
	     vectorPresion2[contadorPosicion] =  (float)(::atof(token.c_str()));
          }
	  cont = cont + 1;

       }
       contadorPosicion = contadorPosicion + 1;
  }


////////////////////////////////////////////////////////////////////////////////

  //Se ejecutan 3 kernels cada uno en un stream diferente y haciendolo en 288 bloques cada uno, de manera aplicar regresion lineal cada 100 datos equivalente a 5 minutos de mediciones para el dia 2
  for(int i=0;i < H;i++){
  // copia de datos hacia el device
  cudaMemcpyAsync(dev_res4, vectorTemperatura2, VUELTAS*sizeof(float), cudaMemcpyHostToDevice,stream4);

  //Se hace la medicion del tiempo atraves de events
  cudaEventRecord(start);
  prediccion<<<1, H>>>(vectorTemperatura2, dev_res4);
  cudaEventRecord(stop);

  cudaMemcpyAsync(res_stream4, dev_res4, VUELTAS*sizeof(float), cudaMemcpyDeviceToHost,stream4);

  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&milliseconds4, start, stop);

/////////////////////////////////////////////////////////////////////////////

  cudaMemcpyAsync(dev_res5, vectorHumedad2, VUELTAS*sizeof(float), cudaMemcpyHostToDevice,stream5);

  //Se hace la medicion del tiempo atraves de events
  cudaEventRecord(start);
  prediccion<<<1, H>>>(vectorHumedad2, dev_res5);
  cudaEventRecord(stop);

  cudaMemcpyAsync(res_stream5, dev_res5, VUELTAS*sizeof(float), cudaMemcpyDeviceToHost, stream5);

  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&milliseconds5, start, stop);


////////////////////////////////////////////////////////////////////////////////

  cudaMemcpyAsync(dev_res6, vectorPresion2, VUELTAS*sizeof(float), cudaMemcpyHostToDevice,stream6);

  //Se hace la medicion del tiempo atraves de events
  cudaEventRecord(start);
  prediccion<<<1, H>>>(vectorPresion2, dev_res6);
  cudaEventRecord(stop);

  cudaMemcpyAsync(res_stream6, dev_res6, VUELTAS*sizeof(float), cudaMemcpyDeviceToHost,stream6);

  cudaEventSynchronize(stop);
  cudaEventElapsedTime(&milliseconds6, start, stop);

  }

///////////////////////////////////////////////////////////////////////////////

  //Se sincronizan los streams
  cudaStreamSynchronize(stream4); // wait for stream1 to finish
  cudaStreamSynchronize(stream5); // wait for stream2 to finish
  cudaStreamSynchronize(stream6); // wait for stream3 to finish

/////////////////////////////////////////////////////////////////////////////////
//Se guardan los datos predecidos en un archivo csv correspondiente
  ofstream Archivo("181113_estCU.csv");
  for(int i=0;i<VUELTAS;i++){
	Archivo << horas[i] << ";" << res_stream1[i] << ";" << res_stream2[i] << ";" << res_stream3[i] << ";" << endl;      
  }
  Archivo.close();

  ofstream Archivo2("181114_estCU.csv");
  for(int i=0;i<VUELTAS;i++){
	Archivo2 << horas2[i] << ";" << res_stream4[i] << ";" << res_stream5[i] << ";" << res_stream6[i] << ";" << endl;      
  }
  Archivo2.close();

  //Se imprimen los tiempos que tardaron cada uno de los kernels
  printf("Tiempo del kernel para la prediccion de temperaturas del dia 1: %f milisegundos\n", milliseconds1);
  printf("Tiempo del kernel para la prediccion de humedades del dia 1: %f milisegundos\n", milliseconds2);
  printf("Tiempo del kernel para la prediccion de presiones del dia 1: %f milisegundos\n", milliseconds3);
  printf("Tiempo del kernel para la prediccion de temperaturas del dia 2: %f milisegundos\n", milliseconds4);
  printf("Tiempo del kernel para la prediccion de humedades del dia 2: %f milisegundos\n", milliseconds5);
  printf("Tiempo del kernel para la prediccion de presiones del dia 2: %f milisegundos\n", milliseconds6);

  //Se destruyen todos los componentes CUDA y se libera la memoria
  cudaEventDestroy(start);
  cudaEventDestroy(stop);

  cudaStreamDestroy(stream1);
  cudaStreamDestroy(stream2);
  cudaStreamDestroy(stream3);
  cudaStreamDestroy(stream4);
  cudaStreamDestroy(stream5);
  cudaStreamDestroy(stream6);

  cudaFree(dev_res1);
  cudaFree(dev_res2);
  cudaFree(dev_res3);
  cudaFree(dev_res4);
  cudaFree(dev_res5);
  cudaFree(dev_res6);

  // salida
  printf("\npulsa INTRO para finalizar...");
  fflush(stdin);
  char tecla = getchar();
  return 0;
}