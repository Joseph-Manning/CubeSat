/*Final script to flash to arduino A
This is the master on the I2C bus
Author Kieran Orr
Contributors
Seb
Joe
Finley*/
//======================================================================//
/*Pin map
5 To PIX GPIO 7
6 To motor driver ENA
7 To motor driver IN1
8 To motor driver IN2
10 Chip select on SD shield
11 MOSI on SD shield
12 MISO on SD shield
13 SCK on SD shield
3.3V on SD shield*/
//======================================================================//
//Libraries
//Light sensing libraries
#include "Adafruit_VEML7700.h" //Light sensor -adafruit
#include "Adafruit_TCS34725.h" //RGB sensor -adafruit

//I2C
#include <Wire.h> //General wire -adafruit
#include <TCA9548A.h> //Multiplexer -Jonathan Dempsey

//Gyro
#include <Adafruit_ICM20948.h> //gyro
//======================================================================//
//Naming
//Gyro
TCA9548A mux;           //multiplexer
Adafruit_ICM20948 icm;  // 9 DOF sensor
//======================================================================//
//Define pins as needed
//Mode switch pin
const int RP = 5;

//motor control
const int FWD = 7; //only need to be H/L
const int BWD = 8; //only need to be H/L
const int ENA = 6;
//======================================================================//
//Data struct
//ICM struc
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp;
//======================================================================//
//Global Variables
//Light sensing
float VL;
double Vrgb;
float VR;
//float VL_;
float Vrgb_;
//float VR_;

// relative angular position of light source
double theta;
// relative distance between light sensors and light source. [m] <- change values of C for [cm]
float distance;
//======================================================================//
//Constants
//Light sensing
//const double R = 0.2345762060349798; // ratio between lux and rgb sensor responses
const float R = 0.5; // value depends on gain
const double C_rgb = 28.803808; // Constant for normalised sensor response with distance
//const float C_lux = 19.528; // lux sensor currently not used for distance, leave here incase
const float beta = 45 * PI / 180; // lux sensor mounting angle in radians
const double denom = R * 2 * sin(beta); // here so it only need to be run once
//const double denom = 3; // here so it only need to be run once
const double n = 1.088025599603545; // fitted cos^n for rgb sensor
const int cutoff = 10; // minimum sensor value before the realtive angle is assumer >90 deg
// Cutoff value subject to further discretion
void setup() {
  //pinModes
  pinMode(RP, INPUT);
  pinMode(FWD, OUTPUT);
  pinMode(BWD, OUTPUT);
  pinMode(ENA, OUTPUT);
  
  //Initialise pins
  digitalWrite(FWD, LOW);
  digitalWrite(BWD, LOW);
  analogWrite(ENA, 0);

  //I2C initialisation
  Wire.begin();
  mux.begin();

  //Wake gyro
  mux.openChannel(0);
  icm.begin_I2C();
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);    //Sets max acceleration rate measureable
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);  // Sets max rotation rate measureable

}
//======================================================================//
void loop() {
  unsigned long RP_val = digitalRead(RP);
  if (RP_val < 990){
  //Light Tracking Operation- Mode 1
  }
  else if (1400 < RP_val < 1600 ){
    //kieran - turn off
  }
  else if (RP_val > 1800){
    //seb -leave gyro for now
  }
  else {
    //leave for now
  }
}
