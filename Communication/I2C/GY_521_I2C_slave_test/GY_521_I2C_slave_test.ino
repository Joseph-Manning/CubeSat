/*I2C Arduino to Arduino, with gyro - Master
This is a proof of concept, to be implemented later
Author Kieran Orr*/
#include <Wire.h> //included for I2C communication

//Define address of slave Arduino
#define SLAD 9

//Define address of test gyro
#define GYAD 0x68

//define data store and float
  byte b_0, b_1, b_2, b_3 

  float GY_Z;

//reconstruction function
float ass() {
  byte b[] = {b_0, b_1, b_2, b_3};
  memcpy(&GY_Z, &b[0], sizeof(GY_Z));
  return GY_Z;
}

//define function for data recieved
void receiveEvent() {
  while(0 < Wire.available()) {
    b_0 = Wire.read();
    b_1 = Wire.read();
    b_2 = Wire.read();
    b_3 = Wire.read();
    ass();
  }
  Serial.println("receive Event");
}

void setup() {
  serial.begin(9600);
  Wire.begin(SLAD);
  Wire.onReceive(receiveEvent);


}

void loop() {
  delay(500);
  serial.println(GY_Z);
}
