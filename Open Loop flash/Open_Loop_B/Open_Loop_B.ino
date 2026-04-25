/*Open loop script to flash to arduino B
This is the slave on the I2C bus
Author Kieran Orr
Contributors
Seb
Joe
Finlay*/
//======================================================================//
//Libraries
#include <Wire.h>  //included for I2C communication

//Data logging
#include <SPI.h>
#include <SD.h>
//======================================================================//
//Define address of slave Arduino
#define SLAD 9
//======================================================================//
//pins
//Fan pins
const int T4 = 4;
const int H1 = 5;
const int H2 = 6;

//SD
const int chipSelect = 10;
//======================================================================//
//define consts
//n of data values to save in mode 1 3 data points n = 3
const int n = 1;
//======================================================================//
//define data stores
//assemble on slave side
volatile byte array[6];
volatile bool dataReady = false;

//SD
File dataFile;
//======================================================================//
//define global variables
//data to save
float gyro_z;

//data saving
int counter;
//======================================================================//
//define function for data recieved
void receiveEvent(int check_len) {
  if (check_len == 6) {
      for (int i = 0; i < 6; i++) array[i] = Wire.read();
      dataReady = true;
  } else {
      while (Wire.available()) Wire.read(); // flush stuck data
}

}
//======================================================================//
void setup() {
  Wire.begin(SLAD);
  Wire.onReceive(receiveEvent);
  counter = 0;

  //SD
  SD.begin(chipSelect);
  dataFile = SD.open("datalog1.txt", FILE_WRITE);
}
//======================================================================//
void loop() {
  if (dataReady) {
    dataReady = false;
    if (array[4] == 1 && array[5] == 0) {
      memcpy(&gyro_z, array, 4);
      counter++;
    }
  }
  if (counter == n) {
    //data logging
    // make a string for assembling the data to log:
    String dataString = "";

    // read sensor and append to the string:
    dataString += String(gyro_z);
    dataString += ",";
    unsigned long t = millis();
    dataString += t;
    dataString += ",";
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.flush();
    }
    counter = 0;
  } else {
  }
}
