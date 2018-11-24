// Universidad del Valle de Guatemala
// Programacion de Microprocesadores
// Proyecto 3 - Prediccion de Datos C++
// Andrea Elias 17048
// Kevin Macario 17369
// David Soto 17551

// Basado en el codigo de:
// Christian Medina

// compile with:
// g++ prediccionProyecto3.cpp bme280.h -lwiringPi -pthread -o prediccionProyecto3

//Se incluyen las librerias necesarias
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <wiringPiI2C.h>
#include "bme280.h"
#include <ctime>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <pthread.h>
#include <algorithm>

#define VUELTAS 28800 // 28800 tomas de mediciones equivale a 48 horas de medicion con 3 segundos de intervalo entre cada una

using namespace std;

//Se hace una estructura que contenga los arrays que maneje los datos de temperatura, humedad, presion, hora, nombre del archivo sobre el cual se escribira y una variable control
struct thread_data {
   double datosT[VUELTAS];
   double datosH[VUELTAS];
   double datosP[VUELTAS];
   string datosHoras[VUELTAS];
   string nombreArchivo;
   int control = 0;
};

//Subrutina que realiza la prediccion a traves de la regresion lineal de temperaturas, humedad y presion de cada 30 segundos de un dia
void *prediccionRegresion(void *structura){
 //Se toma la estructura en la subrutina
 struct thread_data *datos = (struct thread_data *)structura; 
   
   //Declaramos la variables para llevar el control de los datos
   int n,i;
   n = 10;
   double sumatoriaT=0;
   double sumatoriaH=0;
   double sumatoriaP=0;
   double sumasDifT=0;
   double sumasDifH=0;
   double sumasDifP=0;
   double potenciaT=0;
   double potenciaH=0;
   double potenciaP=0;
   double pendienteT=0;
   double pendienteH=0;
   double pendienteP=0;
   double nuevaT=0;
   double nuevaH=0;
   double nuevaP=0;

   //Calculamos el intercepto de la regresion que es un promedio de los datos medidos en 30 segundos
   for(i=0;i<n;i++){
      sumatoriaT = sumatoriaT + datos->datosT[datos->control + i];
      sumatoriaH = sumatoriaH + datos->datosH[datos->control + i];
      sumatoriaP = sumatoriaP + datos->datosP[datos->control + i];
   }
   sumatoriaT = sumatoriaT/(double)n;
   sumatoriaH = sumatoriaH/(double)n;
   sumatoriaP = sumatoriaP/(double)n;

   //Calculamos la pendiente de la regresion, que es calcular las diferencias entre los datos con respecto a la media y hacer una desviacion estandar
   for(i=0;i<n;i++){
      sumasDifT = (datos->datosT[datos->control + i] - sumatoriaT);
      sumasDifH = (datos->datosH[datos->control + i] - sumatoriaH);
      sumasDifP = (datos->datosP[datos->control + i] - sumatoriaP);	
   }
   potenciaT = pow(sumasDifT,2.00);
   pendienteT = potenciaT/n;  
   potenciaH = pow(sumasDifH,2.00);
   pendienteH = potenciaH/n; 
   potenciaP = pow(sumasDifP,2.00);
   pendienteP = potenciaP/n;  

   //Se escribe en el archivo de prediccion las predicciones realizadas
   fstream fs;
   for(i=0;i<n;i++){
       nuevaT = sumatoriaT + pendienteT*i;
       nuevaH = sumatoriaH + pendienteH*i;
       nuevaP = sumatoriaP + pendienteP*i;
       fs.open(datos->nombreArchivo,std::fstream::app);
       fs<<datos->datosHoras[datos->control + i]<<";"<<nuevaT<<";"<<nuevaH<<";"<<nuevaP<<";"<<endl;
       fs.close();
   }
}

//Subrutina que calcula la media de cualquier tipo de dato recolectado
void media(float* arreglo, string tipo){
    float sumatoria = 0;
    float med = 0;
    for(int i=0;i<VUELTAS;i++){
  	  sumatoria = sumatoria + arreglo[i];	  
    }
    med = sumatoria/(float) VUELTAS; 
    cout<<"La media de las "<<tipo<<" es: "<<med<<endl;
}

//Subrutina que calcula cual fue la mayor medicion en el dia con hora a la que fue medida
void mayor(float* arreglo, string tipo, string* hora){
    float may=arreglo[0];
    string hor=hora[0];
    for(int i=0;i<VUELTAS;i++)
    { if(arreglo[i]>may){
      may=arreglo[i];
      hor=hora[i];}
             
    }
    cout<<"La lectura mayor de las "<<tipo<<" es: "<<may<<" ,tomada a la hora: "<<hor<<endl;
}

//Subrutina que calcula cual fue la menor medicion en el dia con hora a la que fue medida
void menor(float* arreglo, string tipo, string* hora){
    float men=arreglo[0];
    string hor=hora[0];
    for(int i=0;i<VUELTAS;i++)
    { if(arreglo[i]<men){
      men=arreglo[i];
      hor=hora[i];}
       
    }
    cout<<"La lectura menor de las "<<tipo<<" es: "<<men<<" ,tomada a la hora: "<<hor<<endl;
}

//Inicio del programa
int main(int argv, char* argc[]){

  //Variables varias
  int rc, detachstate;

  //Puntero para returns
  void *miResult;
  pthread_attr_t attr;
  pthread_t tid[VUELTAS];
  double resultados[VUELTAS];

  //Estructuras
  struct thread_data argumentosDia1;
  struct thread_data argumentosDia2;

  //Se abren los archivos y se limpian
  ofstream ArchivoPrediccion("181113_estPI.csv");
  ArchivoPrediccion.close();
  ofstream ArchivoPrediccion2("181114_estPI.csv");
  ArchivoPrediccion2.close();

  // Se le dice al usuario que continue la ejecucion de las primeras predicciones con enter
  printf("\npulsa INTRO para continuar con las predicciones del dia 1...");
  fflush(stdin);
  char tecla = getchar();
  
  // Se leen los datos del dia 1
  ifstream datos("181113.csv");
  string linea;
  float temperaturas[VUELTAS];
  float humedades[VUELTAS];
  float presiones[VUELTAS];
  string horas[VUELTAS];
  int contadorPosicion = 0;
  string::size_type sz;

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
	     temperaturas[contadorPosicion] = stof(token,&sz);
          }
	  if(cont == 2){
	     humedades[contadorPosicion] = stof(token,&sz);
          }
	  if(cont == 3){
	     presiones[contadorPosicion] =  stof(token,&sz);
          }
	  cont = cont + 1;

       }
       contadorPosicion = contadorPosicion + 1;
  }

  // Calcula media de temperaturas
  media(temperaturas,"temperaturas");

  // Calcula media de humedades
  media(humedades,"humedades");

  // Calcula media de presiones
  media(presiones,"presiones");

  // Calcula la mayor de las temperaturas
  mayor(temperaturas,"temperaturas",horas);

  // Calcula la mayor de las humedades
  mayor(humedades,"humedades",horas);

  // Calcula la mayor de las presiones
  mayor(presiones,"presiones",horas);

  // Calcula la menor de las temperaturas
  menor(temperaturas,"temperaturas",horas);

  // Calcula la menor de las humedades
  menor(humedades,"humedades",horas);

  // Calcula la menor de las presiones
  menor(presiones,"presiones",horas);

  //Se guardan los datos en la estructura
  argumentosDia1.nombreArchivo = "181113_estPI.csv";
  for(int i=0; i<VUELTAS; i++){
    argumentosDia1.datosT[i] = temperaturas[i];
    argumentosDia1.datosH[i] = humedades[i];
    argumentosDia1.datosP[i] = presiones[i];
    argumentosDia1.datosHoras[i] = horas[i];
  }

  //Se aplican PTHREADS
  for (int i=0; i<(VUELTAS/10); i++){
     //Se hace la media
        pthread_create(&tid[i], NULL, prediccionRegresion,(void *)&argumentosDia1);
     //Se realiza el Join del Thread y se lleva la variable control para hacer otro thread
	pthread_join(tid[i], &miResult);
        argumentosDia1.control = 10 + argumentosDia1.control;
  }

  // Continuacion del Programa para el dia 2
  printf("\npulsa INTRO para continuar con las predicciones del dia 2...");
  fflush(stdin);
  tecla = getchar();
  
  // Se leen los datos del dia 2
  ifstream datos2("181114.csv");
  contadorPosicion = 0;

  // Se obtienen los datos separados de cada linea guardada
  while(getline(datos2,linea)){
       //cout<<linea<<endl;
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
	     temperaturas[contadorPosicion] = stof(token,&sz);
          }
	  if(cont == 2){
	     humedades[contadorPosicion] = stof(token,&sz);
          }
	  if(cont == 3){
	     presiones[contadorPosicion] =  stof(token,&sz);
          }
	  cont = cont + 1;

       }
       contadorPosicion = contadorPosicion + 1;
  }

  // Calcula media de temperaturas
  media(temperaturas,"temperaturas");

  // Calcula media de humedades
  media(humedades,"humedades");

  // Calcula media de presiones
  media(presiones,"presiones");

  // Calcula la mayor de las temperaturas
  mayor(temperaturas,"temperaturas",horas);

  // Calcula la mayor de las humedades
  mayor(humedades,"humedades",horas);

  // Calcula la mayor de las presiones
  mayor(presiones,"presiones",horas);

  // Calcula la menor de las temperaturas
  menor(temperaturas,"temperaturas",horas);

  // Calcula la menor de las humedades
  menor(humedades,"humedades",horas);

  // Calcula la menor de las presiones
  menor(presiones,"presiones",horas);

  //Se guardan los datos en la estructura
  argumentosDia2.nombreArchivo = "181114_estPI.csv";
  for(int i=0; i<VUELTAS; i++){
    argumentosDia2.datosT[i] = temperaturas[i];
    argumentosDia2.datosH[i] = humedades[i];
    argumentosDia2.datosP[i] = presiones[i];
    argumentosDia2.datosHoras[i] = horas[i];
  }

  //Se aplican PTHREADS
  for (int i=0; i<(VUELTAS/10); i++){
     //Se hace la media
        pthread_create(&tid[i], NULL, prediccionRegresion,(void *)&argumentosDia2);
     //Se realiza el Join del Thread y se lleva la variable control para hacer otro thread
	pthread_join(tid[i], &miResult);
        argumentosDia2.control = 10 + argumentosDia2.control;
  }
 
  return 0;
}
