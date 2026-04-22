/*Final script to flash to arduino A
This is the master on the I2C bus
Author Kieran Orr
Contributors
Seb
Joe
Finley*/
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
#include "Adafruit_VEML7700.h" //Light sensor -adafruit
#include "Adafruit_TCS34725.h" //RGB sensor -adafruit

//I2C
#include <Wire.h> //General wire -adafruit
#include <TCA9548A.h> //Multiplexer -Jonathan Dempsey

//Gyro
#include <Adafruit_ICM20948.h> //gyro
//======================================================================//
//Define Objects
//Gyro
TCA9548A mux;           //multiplexer
Adafruit_ICM20948 icm;  // 9 DOF sensor

//Light sensors
Adafruit_VEML7700 veml = Adafruit_VEML7700(); // lux
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X); // RGB
// RGB very sensitive to interger overflow / saturation -> these values work with my phone flashlight
// the test light may be brighter
// Datasheet recommends dark plastic filter over sensor
//======================================================================//
//Define pins as needed
//Mode switch pin
const int RP = 3;

//motor control
const int ENA = 5;
const int IN1 = 6; //only need to be H/L
const int IN2 = 7; //only need to be H/L

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
//======================================================================//
void setup() {
  //pinModes
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

  //I2C initialisation
  Wire.begin();
  mux.begin();

  //Wake gyro
  mux.openChannel(0);
  icm.begin_I2C();
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);    //Sets max acceleration rate measureable
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);  // Sets max rotation rate measureable

  //Light sensor set up
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
