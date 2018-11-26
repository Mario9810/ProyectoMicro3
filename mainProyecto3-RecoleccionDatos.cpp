// Universidad del Valle de Guatemala
// Programacion de Microprocesadores
// Mario Sarmientos
// Fernando Hengstenberg	17699

//llamamos las librerias a utilizar
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <ctime>
#include <math.h>
#include "bme280.h"
#include <errno.h>
#include <sstream>
#include <stdint.h>
#include <wiringPiI2C.h>
#include <time.h>
#include <string>
#include <vector
//17280 mediciones por dia cada 5 segundos
#define VUELTAS 17280 

using namespace std;

//Inicio del programa
int main(int argv, char* argc[]){

  //Se guarda la informacion del Sensor como setup
  int fd = wiringPiI2CSetup(BME280_ADDRESS);

  //Se realiza la apertura del archivo
  ofstream Fille("181124.csv");

  //Se verifica que el sensor este conectado
  if(fd < 0){
    printf("Device not found");
    return -1;
  }

  //Se crean las variables de calibracion del programa para las mediciones mediante el sensor
  bme280_calib_data cal;
  readCalibrationData(fd, &cal);

  wiringPiI2CWriteReg8(fd, 0xf2, 0x01);   
  wiringPiI2CWriteReg8(fd, 0xf4, 0x25);  
  //Se obtienen los datos medidos por el sensor
  bme280_raw_data raw;
  getRawData(fd, &raw);

  //Se obtienen los valores medidos en una variable que soporta la data
  int32_t t_fine = getTemperatureCalibration(&cal, raw.temperature);

  //Se abre el archivo
  if(Fille.is_open()){
    
	Fille << "TIEMPO,TEMPERATURA,HUMEDAD,PRESION\n";	
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

     wiringPiI2CWriteReg8(fd, 0xf2, 0x01);   
     wiringPiI2CWriteReg8(fd, 0xf4, 0x25);   

	 //Se obtienen datos nuevamente
     getRawData(fd, &raw);
     t_fine = getTemperatureCalibration(&cal, raw.temperature);
     
	 //Se ingresan los datos obtenidos en nuevas variables
     float temperatura = compensateTemperature(t_fine); // C
     
	 float humedad = compensateHumidity(raw.humidity, &cal, t_fine);       // %
     
	 float altura = getAltitude(p);
	 
	 float presion = compensatePressure(raw.pressure, &cal, t_fine) / 100; // hPa
                           
     
     // Se imprime la hora segun el reloj de la raspberry
     cout << tiempo << endl;

    
     printf("{\"sensor\":\"bme280\", \"temperatura\":%.2f, \"presion\":%.2f,"
     " \"humedad\":%.2f, \"altitud\":%.2f, \"tiempo\":%d}\n",
     temperatura, presion, humedad, altitud, (int)time(NULL));

     // Se escriben los datos en el documento
     Fille << tiempo << " | " << presion << " | " << humedad << " | " << temperatura << " | " << endl;
	
     // Lectura de tiempo esperar 3 segundos
     double time = 0;  
     unsigned tiempoF,tiempoO;

     tiempoF=clock();	  
	  
     do{  
  	tiempoO=clock();
  	time = (double(tiempoO-tiempoF)/CLOCKS_PER_SEC); 
        }
	  
	while (int(time)<3);
	  
	}
  //Se cierra el archivo 1
  Fille.close();

  //Segundo dia de lectura
  //Se abre el archivo de escritura para el dia 2
  ofstream FilleT("181117.csv");

  //Se comprueba que este abierto el archivo
  if (FilleT.is_open()) {
	  FilleT << "T |Temp | H |P\n";	
  }
  else {

	  cout << "file couldnt be opened";
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
	  float temperatura= compensateTemperature(t_fine); // C
	  float presion = compensatePressure(raw.pressure, &cal, t_fine) / 100; 
	  float humedad = compensateHumidity(raw.humidity, &cal, t_fine);      
	  float altura = getAltitude(p);                        

	  // Se imprime la hora segun el reloj de la raspberry
	  cout << tiempo << endl;

	  // output data to screen
	  printf("{\"sensor\":\"bme280\", \"temperatura\":%.2f, \"pressure\":%.2f,"  "\"humedad\":%.2f, \"altitude\":%.2f, \"timestamp\":%d}\n",temperatura , presion, humedad, a, (int)time(NULL));
	 // Se escriben los datos en el documento
	 FilleT << tiempo << " | " << presion << " | " << humedad << " | " << temperatura << " | " << endl;

	  // Lectura de tiempo esperar 5 segundos
	  double time = 0;
	  unsigned tiempoF, tiempoO;

	  tiempoF = clock();
	//DO task----------------------------------------------------------
	  do {
		  tiempoO = clock();
		  //calculo de tiempo
		  time = (double(tiempoO - tiempoF) / CLOCKS_PER_SEC);
	  }

	  while (int(time) < 5);

  }
  
  FilleT.close();
  return 0;
}
