/*Open loop script to flash to arduino A
This is the master on the I2C bus
Author Kieran Orr
Contributors
Seb
Joe
Finlay
Navya*/
//======================================================================//
/*Pin map
3 To PIX GPIO 7
5 To motor driver ENA
6 To motor driver IN1
7 To motor driver IN2
9 To Tanslational 1
10 To Translational 2
11 To Translational 3

A4 Data pin (blue I2C connect)
A5 Clock pin (yellow I2C connect)

5V VCC for I2C
GND for I2C*/
//======================================================================//
//Libraries
//Light sensing libraries
#include "Adafruit_VEML7700.h"  //Light sensor -adafruit
#include "Adafruit_TCS34725.h"  //RGB sensor -adafruit

//I2C
#include <Wire.h>      //General wire -adafruit
#include <TCA9548A.h>  //Multiplexer -Jonathan Dempsey

//Gyro
#include <Adafruit_ICM20948.h>  //gyro
//======================================================================//
//Define Objects
//Gyro
TCA9548A mux;           //multiplexer
Adafruit_ICM20948 icm;  // 9 DOF sensor
//======================================================================//
//Define address of slave Arduino
#define SLAD 9
//======================================================================//
//Define pins as needed
//5V logic pin
const int logic = 2;
//Mode switch pin
const int RP = 3;

//motor control
const int ENA = 5;
const int IN1 = 6;  //only need to be H/L
const int IN2 = 7;  //only need to be H/L

//Signal intercept
const int T1 = 9;
const int T2 = 10;
const int T3 = 11;
//======================================================================//
//Data struct
//ICM struc
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp;
//======================================================================//
//Global Variables
//gyro-acc data
float gyro_z;
//======================================================================//
void setup() {

  Serial.begin(9600);
  //pinModes
  pinMode(logic, OUTPUT);
  pinMode(RP, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(T1, INPUT);
  pinMode(T2, INPUT);
  pinMode(T3, INPUT);

  //Initialise pins
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
  digitalWrite(logic, HIGH);

  //I2C initialisation
  Wire.begin();
  mux.begin();

  //Wake gyro
  mux.openChannel(0);
  icm.begin_I2C();
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);    //Sets max acceleration rate measureable
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);  // Sets max rotation rate measureable
  delay(3000);
  //Test pulse
  for (int i = 0; i < 200; i++) {
    digitalWrite(IN1, HIGH);
    analogWrite(ENA, 255);
    //Read gyro
    mux.openChannel(0);
    icm.getEvent(&accel, &gyro, &temp);
    gyro_z = gyro.gyro.z;
    //Send Gyro data
    //process gyro_z to byte array
    byte Gyro_array[6];
    for (int i = 0; i < 4; i++) {
      Gyro_array[i] = ((byte*)(&gyro_z))[i];
    }
    Gyro_array[4] = 1;  //byte ID for slave side ID
    Gyro_array[5] = 0;
    //send gyro_z to slave
    Wire.beginTransmission(SLAD);
    Wire.write(Gyro_array, 6);
    Wire.endTransmission();
    delay(5);
  }
  analogWrite(ENA, 0);
  digitalWrite(IN1, LOW);
}

void loop() {
  Serial.println(gyro_z);
  digitalWrite(IN1, LOW);
  analogWrite(ENA, 140);
  //Read gyro
  mux.openChannel(0);
  icm.getEvent(&accel, &gyro, &temp);
  gyro_z = gyro.gyro.z;
  //Send Gyro data
  //process gyro_z to byte array
  byte Gyro_array[6];
  for (int i = 0; i < 4; i++) {
    Gyro_array[i] = ((byte*)(&gyro_z))[i];
  }
  Gyro_array[4] = 1;  //byte ID for slave side ID
  Gyro_array[5] = 0;
  //send gyro_z to slave
  Wire.beginTransmission(SLAD);
  Wire.write(Gyro_array, 6);
  Wire.endTransmission();
  delay(200);
}
