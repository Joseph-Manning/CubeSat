/*I2C Arduino to Arduino, with gyro - Master
This is a proof of concept, to be implemented later
Author Kieran Orr*/
#include <Wire.h> //included for I2C communication

//Define address of slave Arduino
#define SLAD 9

//Define address of test gyro
#define GYAD 0x68

int16_t rawZ;
float GY_Z;

void setup() {
  Wire.begin(); //I2C communication initiation
  Serial.begin(9600); //for monitering communication - not needed for I2C

  //wake the gyro
  Wire.beginTransmission(GYAD); 
  Wire.write(0x6B);             //power management id
  Wire.write(0);                //sleep set false
  Wire.endTransmission(true);
}

void loop() {
  //Pull gyro z data from GY-521
  Wire.beginTransmission(GYAD);
  Wire.write(0x47);
  Wire.endTransmission(false);
  Wire.requestFrom(GYAD,2,1);
  rawZ = Wire.read() << 8 | Wire.read();
  GY_Z = rawZ/131; //for 250 deg/s defult

  //process GY_Z to byte array
  byte array[4];
  for (int i=0; i<4; i++) {
    array[i] = ((byte*)(&GY_Z))[i];
  }
  //send GY_Z to slave
  Wire.beginTransmission(SLAD);
  Wire.write(array, 4);
  Wire.endTransmission();
  delay(500);
}
