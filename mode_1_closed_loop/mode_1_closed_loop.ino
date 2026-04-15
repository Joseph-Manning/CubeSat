/*Close loop code Mode 1
Load on to the arduino that features the SD card and the RW motor
Needs to interact with PIX
Author:Kieran Orr*/

//Libraries
//Data logging
#include <SPI.h>
#include <SD.h>
//Gyro data reading
#include <Wire.h> //General wire 
#include <TCA9548A.h> //Multiplexer
//#include <Adafruit_ICM20948.h> //Needed for the IMU
#include <Adafruit_ICM20X.h>//Needed for IMU
#include <Adafruit_Sensor.h> //sensor lab to run multiplexer
//Light sensing libraries
#include "Adafruit_VEML7700.h" //Light sensor
#include "Adafruit_TCS34725.h" //RGB sensor

//naming
TCA9548A mux; //multiplexer
Adafruit_ICM20948 icm; // 9 DOF sensor

//pins
const int check_read = 3;
const int chipSelect = 10;

//ICU struc - for imu func
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp;

//Light Constants
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

//SD
File dataFile;

//Global
double dt, last_time, now;

//Kalman filter function
double kalman(double theta_model, double theta_gyro, double desired_value) {
  float K_kalman = 1;
  double theta_measured = theta_model * K_kalman + (1-K_kalman) * theta_gyro;
  double error = desired_value - theta_measured
  return error_output

})

//Define PD function 
double pd(double error, double error_last, double dt) {
  float Kp = 1;
  float Kd = 1;
  double proportional = Kp*error;
  double derivative = Kd*(error-error_last)/dt;
  double output = proportional + derivative;
  return pd_output;
}
void setup() {
  //serial begin
  Serial.begin(115200);
  //multiplex setup
  Wire.begin();
  mux.begin();
  // gyro setup
  mux.openChannel(3);
  icm.begin_I2C();
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G); //Sets max acceleration rate measureable
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS); // Sets max rotation rate measureable
  //SD set up
  SD.begin(chipSelect);
  pinMode(check_read, OUTPUT);
  digitalWrite(check_read, LOW);
  dataFile = SD.open("datalog.txt", FILE_WRITE);
}


void loop() {
  // put your main code here, to run repeatedly:
  double now = millis();
  double dt = now - last_time;
  double last_time = now;
}
