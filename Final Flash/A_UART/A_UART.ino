/* Final script to flash to Arduino A
   MASTER (UART to slave)
   Authors: Kieran, Joe
   Contributors: Seb, Finley
*/
//======================================================================//
// Pin map and libraries remain unchanged
#include <math.h>

// Light sensing
#include "Adafruit_VEML7700.h"
#include "Adafruit_TCS34725.h"

// I2C for sensors only
#include <Wire.h>
#include <TCA9548A.h>

// Gyro
#include <Adafruit_ICM20948.h>

//======================================================================//
// OBJECTS
TCA9548A mux;
Adafruit_ICM20948 icm;

Adafruit_VEML7700 veml = Adafruit_VEML7700();
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X);

//======================================================================//
// Pins
const int logic = 4;
const int RP = 3;
const int ENA = 5;
const int IN1 = 6;
const int IN2 = 7;
const int LED = 8;
const int T1 = 9;
const int T2 = 10;
const int T3 = 11;

//======================================================================//
// UART PACKET SENDER
// [4 bytes float][1 byte id1][1 byte id2]
void sendFloatUART(float f, byte id1, byte id2) {
    Serial.write((byte*)&f, 4);
    Serial.write(id1);
    Serial.write(id2);
}

//======================================================================//
// GLOBAL VARIABLES
unsigned long RP_val;
float theta;

float VL;
double Vrgb;
float VR;

float theta_sen;
float gyro_z;
float gyro_z_mod;
float theta_inertia;
float K_gyro_loop;

float error;
float error_last;
bool Kalman_valid;
float d_theta;

float p;
float d;
float pd_output;
float theta_last;
float time_last;

double dt, last_time, now;

const int n_history = 5;
int index = 0;
float theta_history[n_history] = {10,10,10,10,10};
float d_theta_history[n_history] = {0,0,0,0,0};
float torque_history[n_history] = {0,0,0,0,0};
float error_history[n_history] = {0,0,0,0,0};
float time_history[n_history] = {0,0,0,0,0};

float sample_time = 50.0;

int motor_pwm;
bool flash = false;

//======================================================================//
// CONSTANTS
#define MUX_ADDR 0x70
const float R = 1 / 2.1;
const float beta = 45 * PI / 180;
const double denom = R * 2 * sin(beta);
const double n = 1.088025599603545;
const int cutoff = 300;

const float K_kalman = 0;
const float K_gyro = 0;

const float Kp = 4;
const float Kd = 2;

const float T0 = 0.018 * 9.81;
const int stall_rpm = 251;

//======================================================================//
// SETUP
void selectChannel(uint8_t i) {
  if (i > 3) return;
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {
  Wire.begin();
  Wire.setWireTimeout(1000, true);

  Serial.begin(115200);   // UART to slave

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

  // Gyro
  selectChannel(0);
  icm.begin_I2C();
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);

  // Lux sensors
  selectChannel(1);
  veml.begin();

  selectChannel(3);
  veml.begin();

  // RGB
  selectChannel(2);
  tcs.begin();

  motor_pwm = 0;
}

//======================================================================//
// MAIN LOOP
void loop() {
  delay(10);

  RP_val = pulseIn(RP, HIGH, 30000);
  RP_val = 900;

  if (RP_val < 990) {

    //==================== GYRO ====================
    selectChannel(0);
    sensors_event_t accel, gyro, Temp;
    icm.getEvent(&accel, &gyro, &Temp);

    gyro_z = -gyro.gyro.z;

    // UART SEND: Gyro (ID = 1,0)
    sendFloatUART(gyro_z, 1, 0);

    //==================== LUX LEFT ====================
    selectChannel(1);
    VL = veml.readLux();
    Serial.print("Lux1:"); Serial.print(VL/1000); Serial.print(",");

    //==================== RGB ====================
    selectChannel(2);
    uint16_t r,g,b,c;
    tcs.getRawData(&r,&g,&b,&c);
    if (g == 0) g = 1;
    Vrgb = tcs.calculateLux(r,g,b);
    if (Vrgb < 1) Vrgb = 10;
    Serial.print("Vrgb:"); Serial.print(Vrgb/1000); Serial.print(",");

    //==================== LUX RIGHT ====================
    selectChannel(3);
    VR = veml.readLux();
    Serial.print("Lux3:"); Serial.print(VR/1000); Serial.print(",");

    //==================== MODE LOGIC ====================
    if (VL < cutoff && VR < cutoff) {
      digitalWrite(LED, LOW);
      d = 0;
      theta_sen = 10;
      p = (VL > VR) ? 3*PI/2 : -3*PI/2;
    }
    else if (!(VR > cutoff && VL > cutoff)) {
      digitalWrite(LED, flash);
      flash = !flash;
      d = 0;
      theta_sen = 10;
      p = (VL > VR) ? PI/2 : -PI/2;
    }
    else {
      digitalWrite(LED, HIGH);
      if (Vrgb == 0) Vrgb = 0.1;
      theta_sen = atan((VL - VR) / (denom * Vrgb));

      Serial.print("theta_sen:");
      Serial.print(theta_sen == 10 ? 0 : theta_sen);
      Serial.print(",");

      dt = millis() - time_last;
      time_last = millis();

      p = theta_sen;
      d = (theta_sen - theta_last) / dt;
      theta_last = theta_sen;
    }
    float error = 3;
    // UART SEND: Theta (ID = 0,0)
    sendFloatUART(theta_sen, 0, 0);

    sendFloatUART(error, 0, 1);

    //==================== MOTOR CONTROL ====================
    motor_pwm += Kp*p + Kd*d;

    if (motor_pwm > 255) motor_pwm = 255;
    if (motor_pwm < -255) motor_pwm = -255;

    if (motor_pwm > 0) {
      digitalWrite(IN1, HIGH);
      digitalWrite(IN2, LOW);
    } else {
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, HIGH);
    }

    analogWrite(ENA, abs(motor_pwm));

    Serial.print("p:"); Serial.print(p);
    Serial.print(",d:"); Serial.print(d);
    Serial.print(",PWM:"); Serial.print(motor_pwm);

    // UART SEND: Motor PWM (ID = 1,1)
    sendFloatUART(motor_pwm, 1, 1);

  }

  Serial.print("Theta:");
  Serial.println(theta == 10 ? 0 : theta * 180 / PI);
}
