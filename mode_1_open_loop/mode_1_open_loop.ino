/*Open loop code Mode 1
Load on to the arduino that features the SD card and the motor
Does not need to interact with the PIX
Author:Kieran Orr*/

//Libraries
//Data logging
#include <SPI.h>
#include <SD.h>
//Gyro data reading
#include <Wire.h> //I2C 
#include <TCA9548A.h> //multiplexer
#include <Adafruit_ICM20948.h> //gyro acc
//#include <Adafruit_ICM20X.h> //might not need this 
//#include <Adafruit_Sensor.h> //optional to make change of sensors easier and data read out
//Arduino to Pix
#include <SoftwareSerial.h>
//naming
TCA9548A mux;           //multiplexer
Adafruit_ICM20948 icm;  // 9 DOF sensor

//pins
const int pulse = 9;
const int check_read = 3;
const int chipSelect = 10;

//ICM struc
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp;
//SD
File dataFile;

void setup() {
  //for gyro
  Wire.begin();
  mux.begin();
  mux.openChannel(0);
  icm.begin_I2C();
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);    //Sets max acceleration rate measureable
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);  // Sets max rotation rate measureable
  //SD
  SD.begin(chipSelect);
  pinMode(check_read, OUTPUT);
  digitalWrite(check_read, LOW);
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  //pulse
  analogWrite(pulse, 255);
  delay(1000);
  analogWrite(pulse, 0);
}

void loop() {
  //data collecting
  mux.openChannel(0);
  icm.getEvent(&accel, &gyro, &temp);

  //data logging
  // make a string for assembling the data to log:
  String dataString = "";

  // read sensor and append to the string:
  float gyro_z = gyro.gyro.z;
  dataString += String(gyro_z);


  // if the file is available, write to it:
  if (dataFile) {
    unsigned long t = millis();
    dataFile.println(dataString);
    dataFile.println(",");
    dataFile.println(t);
    dataFile.println(",");
  } else {
    //as this is not connected to pc use led to check
    digitalWrite(check_read, HIGH);
  }
  delay(50);  //The delay between loops
}
