/*I2C Arduino to Arduino, with SD card - Slave
This is a proof of concept, to be implemented later
Author Kieran Orr*/
#include <Wire.h>  //included for I2C communication
#include <SPI.h>
#include <SD.h>

//Define address of slave Arduino
#define SLAD 9

//string for data
String dataString = "";

//data store
volatile bool dataReady = 0;

//SD
const int chipSelect = 10;
File dataFile;

//define function for data recieved
void receiveEvent(int ansLen) {
  if (ansLen == 15 && dataReady == 0) {
    while (Wire.available()) {
      char b = Wire.read();
      dataString += b;
    }
    dataReady = 1;
  }
}

void setup() {
  Wire.begin(SLAD);
  Wire.onReceive(receiveEvent);

  //serial to check
  Serial.begin(9600);

  //SD open
  //SD
  SD.begin(chipSelect);
  dataFile = SD.open("cumFile.txt", FILE_WRITE);
}

void loop() {
  //print data if ready
  if (dataReady == 1) {
    Serial.println(dataString);
    if (dataFile) {
      noInterrupts();
      dataFile.println(dataString);
      dataFile.flush();
      interrupts();
    }
    dataReady = 0;
    dataString = "";
  }
}
