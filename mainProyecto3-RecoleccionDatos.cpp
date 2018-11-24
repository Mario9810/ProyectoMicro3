// Universidad del Valle de Guatemala
// Programacion de Microprocesadores
// Main-Proyecto 3
// Andrea Elias 17048
// Kevin Macario 17369
// David Soto 17551

// Basado en el codigo de:
// Christian Medina

// compile with:
// g++ mainProyecto3-RecoleccionDatos.cpp bme280.cpp bme280.h -lwiringPi -o Proyecto3-Recoleccion

//Se indican las librerias necesarias para el programa
#include <iostream>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <wiringPiI2C.h>
#include "bme280.h"
#include <ctime>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#define VUELTAS 28800 // 28800 tomas de mediciones equivale a 1 dia de mediciones con 3 segundos de intervalo entre cada una

using namespace std;

//Inicio del programa
int main(int argv, char* argc[]){

  //Se guarda la informacion del Sensor como setup
  int fd = wiringPiI2CSetup(BME280_ADDRESS);

  //Se realiza la apertura del archivo
  ofstream Archivo("181113.csv");

  //Se verifica que el sensor este conectado
  if(fd < 0){
    printf("Device not found");
    return -1;
  }

  //Se crean las variables de calibracion del programa para las mediciones mediante el sensor
  bme280_calib_data cal;
  readCalibrationData(fd, &cal);

  wiringPiI2CWriteReg8(fd, 0xf2, 0x01);   // humidity oversampling x 1
  wiringPiI2CWriteReg8(fd, 0xf4, 0x25);   // pressure and temperature oversampling x 1, mode normal

  //Se obtienen los datos medidos por el sensor
  bme280_raw_data raw;
  getRawData(fd, &raw);

  //Se obtienen los valores medidos en una variable que soporta la data
  int32_t t_fine = getTemperatureCalibration(&cal, raw.temperature);

  //Se abre el archivo
  if(Archivo.is_open()){
    ////Archivo << "TIEMPO,TEMPERATURA,HUMEDAD,PRESION\n";	
  } else {
	
     cout << "ERROR: No se puedo realizar la apertura del archivo";
     return 0;		
  }
  
  //Se hace la recoleccion de datos y escritura en archivos durante un dia
  for(int i = 0; i < VUELTAS; i++){

     // Se hace un calculo del tiempo en horas y minutos
     time_t theTime = time(NULL);
     struct tm *aTime = localtime(&theTime);
     int hour=aTime->tm_hour;
     int min=aTime->tm_min;	
	
     // Se crea un string con del tiempo
     stringstream ss;
     ss << hour << ":" << min;
     string tiempo = ss.str();

	 //Se recalibran los datos
     readCalibrationData(fd, &cal);

     wiringPiI2CWriteReg8(fd, 0xf2, 0x01);   // humidity oversampling x 1
     wiringPiI2CWriteReg8(fd, 0xf4, 0x25);   // pressure and temperature oversampling x 1, mode normal

	 //Se obtienen datos nuevamente
     getRawData(fd, &raw);
     t_fine = getTemperatureCalibration(&cal, raw.temperature);

	 //Se ingresan los datos obtenidos en nuevas variables
     float t = compensateTemperature(t_fine); // C
     float p = compensatePressure(raw.pressure, &cal, t_fine) / 100; // hPa
     float h = compensateHumidity(raw.humidity, &cal, t_fine);       // %
     float a = getAltitude(p);                         // meters
     
     // Se imprime la hora segun el reloj de la raspberry
     cout << tiempo << endl;

     // output data to screen
     printf("{\"sensor\":\"bme280\", \"humidity\":%.2f, \"pressure\":%.2f,"
     " \"temperature\":%.2f, \"altitude\":%.2f, \"timestamp\":%d}\n",
     h, p, t, a, (int)time(NULL));

     // Se escriben los datos en el documento
     Archivo << tiempo << ";" << t << ";" << h << ";" << p << ";" << endl;
	
     // Lectura de tiempo esperar 3 segundos
     double time = 0;  
     unsigned t0,t1;

     t0=clock();	  
	  
     do{  
  	t1=clock();
  	time = (double(t1-t0)/CLOCKS_PER_SEC); 
        }
	  
	while (int(time)<3);
	  
	}
  //Se cierra el archivo 1
  Archivo.close();

  //Segundo dia de lectura
  //Se abre el archivo de escritura para el dia 2
  ofstream Archivo2("181114.csv");

  //Se comprueba que este abierto el archivo
  if (Archivo2.is_open()) {
	  ////Archivo << "TIEMPO,TEMPERATURA,HUMEDAD,PRESION\n";	
  }
  else {

	  cout << "ERROR: No se puedo realizar la apertura del archivo";
	  return 0;
  }

  //Se inicia la recoleccion de datos y escritura durante un dia
  for (int i = 0; i < VUELTAS; i++) {

	  // Se hace un calculo del tiempo en horas y minutos
	  time_t theTime = time(NULL);
	  struct tm *aTime = localtime(&theTime);
	  int hour = aTime->tm_hour;
	  int min = aTime->tm_min;

	  // Se crea un string con del tiempo
	  stringstream ss;
	  ss << hour << ":" << min;
	  string tiempo = ss.str();

	  //Se recalibran los datos
	  readCalibrationData(fd, &cal);

	  wiringPiI2CWriteReg8(fd, 0xf2, 0x01);   // humidity oversampling x 1
	  wiringPiI2CWriteReg8(fd, 0xf4, 0x25);   // pressure and temperature oversampling x 1, mode normal

	  //Se obtienen los datos
	  getRawData(fd, &raw);
	  t_fine = getTemperatureCalibration(&cal, raw.temperature);

	  //Se crean las variables para recibir los datos
	  float t = compensateTemperature(t_fine); // C
	  float p = compensatePressure(raw.pressure, &cal, t_fine) / 100; // hPa
	  float h = compensateHumidity(raw.humidity, &cal, t_fine);       // %
	  float a = getAltitude(p);                         // meters

	  // Se imprime la hora segun el reloj de la raspberry
	  cout << tiempo << endl;

	  // output data to screen
	  printf("{\"sensor\":\"bme280\", \"humidity\":%.2f, \"pressure\":%.2f,"
		  " \"temperature\":%.2f, \"altitude\":%.2f, \"timestamp\":%d}\n",
		  h, p, t, a, (int)time(NULL));

	  // Se escriben los datos en el documento
	  Archivo2 << tiempo << ";" << t << ";" << h << ";" << p << ";" << endl;

	  // Lectura de tiempo esperar 3 segundos
	  double time = 0;
	  unsigned t0, t1;

	  t0 = clock();

	  do {
		  t1 = clock();
		  time = (double(t1 - t0) / CLOCKS_PER_SEC);
	  }

	  while (int(time) < 3);

  }
  //Se cierra el archivo 2
  Archivo2.close();
  return 0;
}
