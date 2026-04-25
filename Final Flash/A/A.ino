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

//Light sensors
Adafruit_VEML7700 veml = Adafruit_VEML7700();                                                 // lux
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X);  // RGB
// RGB very sensitive to interger overflow / saturation -> these values work with my phone flashlight
// the test light may be brighter
// Datasheet recommends dark plastic filter over sensor
//======================================================================//
//Define address of] slave Arduino
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

//LED pin
const int LED = 8;

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
//Mode ID
unsigned long RP_val;
//Light sensing
float VL;
double Vrgb;
float VR;
//float VL_;
float Vrgb_;
//float VR_;

// relative angular position of light source
float theta_sen;

// relative distance between light sensors and light source. [m] <- change values of C for [cm]
float distance;

//gyro-acc data
float gyro_z;

//model variables
float gyro_z_mod;

//kalman filter/PD
float error;
float error_last;

//time
double dt, last_time, now;

//motor command
int motor_pwm;
//======================================================================//
//Constants
//Light sensing
//const double R = 0.2345762060349798; // ratio between lux and rgb sensor responses
const float R = 0.5;             // value depends on gain
const double C_rgb = 28.803808;  // Constant for normalised sensor response with distance
//const float C_lux = 19.528; // lux sensor currently not used for distance, leave here incase
const float beta = 45 * PI / 180;        // lux sensor mounting angle in radians
const double denom = R * 2 * sin(beta);  // here so it only need to be run once
//const double denom = 3; // here so it only need to be run once
const double n = 1.088025599603545;  // fitted cos^n for rgb sensor
const int cutoff = 500;              // minimum sensor value before the realtive angle is assumer >90 deg
// Cutoff value subject to further discretion

//Kalman const
float K_kalman = 1;

//PD const
float Kp = 1;
float Kd = 1;
//======================================================================//
void setup() {
  //pinModes
  pinMode(logic, OUTPUT);
  pinMode(RP, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(T1, INPUT);
  pinMode(T2, INPUT);
  pinMode(T3, INPUT);

  // Initialise motor off
  motor_pwm = 0;
  //Initialise pins
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, motor_pwm);
  digitalWrite(logic, HIGH);

  //I2C initialisation
  Wire.begin();
  mux.begin();

  //Wake gyro
  mux.openChannel(0);
  icm.begin_I2C();
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);    //Sets max acceleration rate measureable
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);  // Sets max rotation rate measureable

  //Light sensor set up
  mux.openChannel(1);
  veml.begin();  //initialise LUX

  mux.openChannel(2);
  tcs.begin();
  tcs.setInterrupt(0);  //turn RGB LED off

  mux.openChannel(3);
  veml.begin();  //initialise LUX

  delay(3000);  //let it turn on
}
//======================================================================//
void loop() {
  RP_val = pulseIn(RP, HIGH);
  if (RP_val < 990) {
    //Light Tracking Operation- Mode 1
    //Light sensing
    mux.openChannel(1);
    VL = veml.readLux();

    mux.openChannel(2);
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);
    // for the calculate lux function, if one of the values is zero (typically green)
    // the normalisation does a div 0 and Vrgb goes to its maximum.
    // Check if g is zero and add 1
    if (g == 0) { g = 1; }
    Vrgb = tcs.calculateLux(r, g, b);  // RGB sensor

    mux.openChannel(3);
    VR = veml.readLux();
    //======================================================================//
    //respond to light readings
    //logic to take optimal path
    /*if both are in cutoff region, check which is closer.
    if equal then should be 180 from source, turn anticlockwise. may be sensitive to ambient
    turn right*/
    if (VL < cutoff && VR < cutoff) {
      if (VL < VR) {
        //need to move clockwise - 
        theta_sen = 10;
        distance = 0;
        motor_pwm = 180;
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        analogWrite(ENA, motor_pwm);

      } else if (VR < VL) {
        //turn anticlockwise
        theta_sen = 10;
        distance = 0;
        motor_pwm = 180;
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        analogWrite(ENA, motor_pwm);
      } else {
        //turn anticlockwise
        theta_sen = 10;
        distance = 0;
        motor_pwm = 180;
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        analogWrite(ENA, motor_pwm);
      }
    }
    //only one sensor in cut off region - run at lower speed
    else if (VR < cutoff && VL > cutoff) {
      //turn anticlockwise
      theta_sen = 10;
      distance = 0;
      motor_pwm = 100;
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
      analogWrite(ENA, motor_pwm);
    } else if (VL < cutoff && VR > cutoff) {
      // Turn clockwise
      theta_sen = 10;
      distance = 0;
      motor_pwm = 100;
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      analogWrite(ENA, motor_pwm);
    }
    //============================================================================//

    else {
      //in PD fine control region now - theta is calculated
      //should only run if all previous conditions false
      // in view of L and R -> in view of rgb
      // calc theta and distance
      theta_sen = atan((VL - VR) / (denom * Vrgb));  // value for theta in radians
      // Find normalised sensor values for distance calc
      // If cos = 0 then div 0
      // cos 0 -> light out of FOV, this code block shouldnt run
      // Only considering distance from rgb sensor for now,
      // until lux sensor accuracy can be improved
      //VL_ = VL / cos(theta + beta);
      Vrgb_ = Vrgb / pow(cos(theta_sen), n);  // power is computational intensive, but huge boost to accuracy
      //VR_ = VR / cos(theta - beta);

      distance = C_rgb / pow(Vrgb_, 0.5);
    }

    //============================================================================//
    //Transmit theta_sen 
    //process theta_sen to byte array
    byte theta_sen_array[6];
    for (int i = 0; i < 8; i++) {
      theta_sen_array[i] = ((byte*)(&theta_sen))[i];
    }
    //data ID 00 for light sensing theta
    theta_sen_array[4] = 0;
    theta_sen_array[5] = 0;
    //send theta_sen to slave
    Wire.beginTransmission(SLAD);
    Wire.write(theta_sen_array, 6);
    Wire.endTransmission();
    //============================================================================//
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
    Gyro_array[4] = 1;  //byte ID for slave side ID 01
    Gyro_array[5] = 0;
    //send gyro_z to slave
    Wire.beginTransmission(SLAD);
    Wire.write(Gyro_array,  6);
    Wire.endTransmission();
    //============================================================================//
    //Model
    //Write system model for kalman filter
    if (theta_sen != 10) {
      //will pass running the model if in a search mode
      //WRONG DICKHEAD you need to clean the gyro data and send the improvement
      //so fix above as well, send on same address
      //Write model here
    }
    //============================================================================//
    //Kalman Filter
    //result in error
    //============================================================================//
    //Transmit error
    //process error to byte array
    byte error_array[6];
    for (int i = 0; i < 4; i++) {
      error_array[i] = ((byte*)(&error))[i];
    }
    error_array[4] = 0;
    error_array[5] = 1;
    //send error to slave
    Wire.beginTransmission(SLAD);
    Wire.write(error_array, 6);
    Wire.endTransmission();
    //============================================================================//
    //PD output
    double now = millis();
    double dt = now - last_time;
    double last_time = now;
    float proportional = Kp * error;
    float derivative = Kd * (error - error_last) / dt;
    float pd_output = proportional + derivative;
    error_last = error;
    //============================================================================//
    //Wrap the pd_output to motor command
    //============================================================================//
    //execute based on direction and pwm command (read if error was negitive, make positive for pd)
    //============================================================================//
    //Transmit motor command
    byte motor_pwm_array[6];
    for (int i = 0; i < 4; i++) {
      error_array[i] = ((byte*)(&motor_pwm))[i];
    }
    error_array[4] = 1;
    error_array[5] = 1;
    //send error to slave
    Wire.beginTransmission(SLAD);
    Wire.write(error_array, 6);
    Wire.endTransmission();
    //============================================================================//
  } 
  else if (1400 < RP_val < 1600) {
    //no mode
    //kill motor
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
  } else if (RP_val > 1800) {
    //seb -leave gyro for now
  } else {
    //leave for now
  }
}
