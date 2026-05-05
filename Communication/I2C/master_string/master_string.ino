/*I2C Arduino to Arduino, using premade string - Master
This is a proof of concept, to be implemented later
Author Kieran Orr*/
#include <Wire.h>  //included for I2C communication

//Define address of slave Arduino
#define SLAD 9

//define data string
String dataString = "abc,cum,cum,cum";
int ansLen = 15;

//define pins
const int handshake = 8;

void setup() {
  Wire.begin();  //I2C com
  Serial.begin(9600);
  pinMode(handshake, INPUT);
}

void loop() {
  //assemble string

  //put string into array
  byte stringArray[ansLen];
  for (int i = 0; i < ansLen; i++) {
    stringArray[i] = dataString.charAt(i);
  }
  //send string
  bool cts = digitalRead(handshake);
  if (cts == 1) {
    Wire.beginTransmission(SLAD);
    Wire.write(stringArray, ansLen);
    Wire.endTransmission();
    Serial.println(dataString);
    delay(50);
  }
}
