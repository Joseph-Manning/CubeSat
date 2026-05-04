/*I2C Arduino to Arduino, using premade string - Master
This is a proof of concept, to be implemented later
Author Kieran Orr*/
#include <Wire.h> //included for I2C communication

//Define address of slave Arduino
#define SLAD 9

//define data string
String dataString = "cum,cum,cum,cum";
int ansLen = 15;

void setup() {
  Wire.begin(); //I2C com

}

void loop() {
  //assemble string

  //put string into array
  byte stringArray[ansLen];
  for (int i = 0; i < ansLen; i++) {
    stringArray[i] =(byte*)dataString.charAt(i);
  }
  //send string
  Wire.beginTransmission(SLAD);
  Wire.write(stringArray, ansLen);
  Wire.endTransmission();
  delay(50);
  Serial.println(dataString);
  
}
