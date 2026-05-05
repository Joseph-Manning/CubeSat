/*Final script to flash to arduino A
This is the master on the I2C bus
Author Kieran Orr
Contributors
Seb
Joe
Finley*/
//======================================================================//
/*Pin map
2 To LED
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
#include "Adafruit_VEML7700.h"
#include <Wire.h>
#include <TCA9548A.h>
#include <Adafruit_ICM20948.h>
#include <math.h>

//======================================================================//
//Define Objects
TCA9548A mux;
Adafruit_ICM20948 icm;
Adafruit_VEML7700 veml = Adafruit_VEML7700();

//======================================================================//
#define SLAD 9

//Pins
const int logic = 2;
const int RP = 3;
const int ENA = 5;
const int IN1 = 6;
const int IN2 = 7;
const int LED = 8;
const int T1 = 9;
const int T2 = 10;
const int T3 = 11;

//======================================================================//
//Data structs
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp;

//======================================================================//
//Globals
unsigned long RP_val;
float VL, VR;
float theta_sen;
float gyro_z;
float error, error_last;
double dt, last_time, now;
int motor_pwm;

//======================================================================//
//Constants
const int cutoff = 500;
float Kp = 1;
float Kd = 1;

//======================================================================//
void setup() {
  pinMode(logic, OUTPUT);
  pinMode(RP, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(T1, INPUT);
  pinMode(T2, INPUT);
  pinMode(T3, INPUT);

  motor_pwm = 0;
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, motor_pwm);
  digitalWrite(logic, HIGH);

  Wire.begin();
  mux.begin();

  mux.openChannel(0);
  icm.begin_I2C();
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);

  mux.openChannel(1);
  veml.begin();

  mux.openChannel(3);
  veml.begin();

  delay(3000);
}

//======================================================================//
void loop() {

  RP_val = pulseIn(RP, HIGH, 30000);

  // ===================== MODE: TRACKING =====================
  if (RP_val < 990) {

    // Read sensors
    mux.openChannel(1);
    VL = veml.readLux();

    mux.openChannel(3);
    VR = veml.readLux();

    // ===================== MODE 1: SEARCH =====================
    if (VL < cutoff && VR < cutoff) {
      theta_sen = 10;
      motor_pwm = 60;
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
      analogWrite(ENA, motor_pwm);
    }

    // ===================== MODE 2 & 3 =====================
    else {

      float sum = VL + VR;
      if (sum < 1) sum = 1;

      theta_sen = (VL - VR) / sum;
    }

    // ===================== SEND THETA =====================
    byte theta_sen_array[6];
    memcpy(theta_sen_array, &theta_sen, 4);
    theta_sen_array[4] = 0;
    theta_sen_array[5] = 0;
    Wire.beginTransmission(SLAD);
    Wire.write(theta_sen_array, 6);
    Wire.endTransmission();

    // ===================== READ GYRO =====================
    mux.openChannel(0);
    icm.getEvent(&accel, &gyro, &temp);
    gyro_z = gyro.gyro.z;

    // SEND GYRO
    byte Gyro_array[6];
    memcpy(Gyro_array, &gyro_z, 4);
    Gyro_array[4] = 1;
    Gyro_array[5] = 0;
    Wire.beginTransmission(SLAD);
    Wire.write(Gyro_array, 6);
    Wire.endTransmission();

    // ===================== ERROR =====================
    if (theta_sen != 10) error = theta_sen;

    byte error_array[6];
    memcpy(error_array, &error, 4);
    error_array[4] = 0;
    error_array[5] = 1;
    Wire.beginTransmission(SLAD);
    Wire.write(error_array, 6);
    Wire.endTransmission();

    // ===================== PD CONTROL =====================
    if (theta_sen != 10) {

      now = millis();
      dt = now - last_time;
      last_time = now;

      float proportional = Kp * error;
      float derivative = 0;
      float pd_output = proportional + derivative;

      error_last = error;

      motor_pwm = abs(pd_output);
      motor_pwm = constrain(motor_pwm, 0, 255);

      if (pd_output > 0) {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
      } else {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
      }
      analogWrite(ENA, motor_pwm);

      // SEND MOTOR PWM
      byte motor_pwm_array[6];
      memcpy(motor_pwm_array, &motor_pwm, 4);
      motor_pwm_array[4] = 1;
      motor_pwm_array[5] = 1;
      Wire.beginTransmission(SLAD);
      Wire.write(motor_pwm_array, 6);
      Wire.endTransmission();
    }
  }

  // ===================== MODE: NEUTRAL =====================
  else if (RP_val > 1400 && RP_val < 1600) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
  }

  // ===================== MODE: SPARE =====================
  else if (RP_val > 1800) {
    // reserved
  }

  // ===================== MODE: DEFAULT =====================
  else {
    // do nothing
  }
}
