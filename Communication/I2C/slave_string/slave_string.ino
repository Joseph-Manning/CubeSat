/*I2C Arduino to Arduino, with SD card - Slave
This is a proof of concept, to be implemented later
Author Kieran Orr*/
#include <Wire.h>  //included for I2C communication
#include <SPI.h>
#include <SD.h>

//Define address of slave Arduino
#define SLAD 9

int freeMemory() {
  char top;
  extern char *__brkval, __bss_end;
  return __brkval ? &top - __brkval : &top - &__bss_end;
}

//string for data
String dataString = "";

//data store
volatile bool dataReady = 0;

//SD
const int chipSelect = 10;
File dataFile;

//define pins
const int handshake = 8;

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
  pinMode(handshake, OUTPUT);
  digitalWrite(handshake, 1);
  dataReady = 0;
  Wire.begin(SLAD);
  Wire.onReceive(receiveEvent);

  //serial to check
  Serial.begin(9600);
  delay(200);

  //SD open
  SD.begin(chipSelect);
  dataFile = SD.open("cock.txt", FILE_WRITE);
  delay(200);
}

void loop() {
  //print data if ready
  if (dataReady == 1) {
    Wire.end();
    digitalWrite(handshake, 0);
    Serial.println("data ready");
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.flush();
      Serial.println(dataString);
      dataString = "";
      dataReady = 0;
      digitalWrite(handshake, 1);
      Wire.begin(SLAD);
    }
  }
  else{
    Serial.println("data not ready");
    Serial.println(dataString);
    delay(1000);
  }
  Serial.println(freeMemory());
}
