#include <Wire.h>

#define SLAVE_ADD 9 //define slave address

#define ANSWERSIZE 5 //slave answer size

void setup() {
  Wire.begin(); //I2C com

  //start serial
  Serial.begin(9600);
}

void loop() {
  delay(5000);
  Serial.println("write to slave");
  
  //write to slave
  Wire.beginTransmission(SLAVE_ADD);
  Wire.write(0);
  Wire.endTransmission();

  //recieve data
  Serial.println("data from slave");
  Wire.requestFrom(SLAVE_ADD, ANSWERSIZE);

  String response = "";
  while (Wire.available()) {
    char b = Wire.read();
    response += b;
  }
  Serial.println(response);
}
