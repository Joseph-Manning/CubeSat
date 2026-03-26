//Open loop code
//Author:Kieran Orr

//Libraries
//Data logging
#include <SPI.h>
#include <SD.h>
//Gyro data reading
#include <Wire.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>

// 
double dt, last_time, now;
//pins
const int pulse = 9;
const int check_read = 3;
//Define PD function ignore for open loop parameter estimation
double pd(double error, double error_last, double dt) {
  float Kp = 1;
  float Kd = 1;
  double proportional = Kp*error;
  double derivative = Kd*(error-error_last)/dt;
  double output = proportional+derivative;
  return output;
}
void setup() {
  double last_time = 0;
  Wire.begin(); //for gyro

  analogWrite(pulse, 255);
  delay(1000);
  analogWrite(pulse, 0);
}

void loop() {
  double now = millis();
  double dt = now - last_time;
  double last_time = now;
  //data logging
  // make a string for assembling the data to log:
  String dataString = "";

  // read sensor and append to the string:
  for (int analogPin = 0; analogPin < 1; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 0) {
      dataString += ",";
    }
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  }
  else{
   //as this is not connected to pc use led to check
   digitalWrite(check_read, HIGH);
  }
}
