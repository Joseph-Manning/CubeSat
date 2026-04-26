/*Final script to flash to arduino A
This is the master on the I2C bus
Authors Kieran, Joe
Contributors
Seb
Finley*/
//======================================================================//
/*Pin map
3 To PIX GPIO 7
4 LOGIC
5 To motor driver ENA
6 To motor driver IN1
7 To motor driver IN2
8 To LED
9 To Tanslational 1
10 To Translational 2
11 To Translational 3

A4 Data pin (blue I2C connect)
A5 Clock pin (yellow I2C connect)

5V VCC for I2C
GND for I2C*/
//================================ LIBRARIES ======================================//
#include <math.h>

// Light sensing
#include "Adafruit_VEML7700.h"  //Light sensor -adafruit
#include "Adafruit_TCS34725.h"  //RGB sensor -adafruit

// I2C
#include <Wire.h>      //General wire -adafruit
#include <TCA9548A.h>  //Multiplexer -Jonathan Dempsey

// Gyro
#include <Adafruit_ICM20948.h>  //gyro

//================================= OBJECTS =======================================//
TCA9548A mux;           //multiplexer
Adafruit_ICM20948 icm;  // 9 DOF sensor

//Light sensors
Adafruit_VEML7700 veml = Adafruit_VEML7700();                                                 // lux
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X);  // RGB

//================================= ADRESSES ======================================//
//Define address of slave Arduino
#define SLAD 9

//Define pins as needed
//5V logic pin
const int logic = 4;

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

//================================ DATA STRUCTURES ==============================//
//Data struct
//ICM struc
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t Temp;

//================================ GLOBAL VARIABLES =============================//

//Mode ID
unsigned long RP_val;
float theta;

//Light sensor values
float VL;
double Vrgb;
float VR;

float theta_sen;      // relative angular position of light source [rad]
float gyro_z;         //gyro-acc data
float gyro_z_mod;     //model variables
float theta_inertia;  // predicted theta based on gyro value

//kalman filter/PD
float error;
float error_last;
bool Kalman_valid;
float d_theta;

float P;
float D;
float pd_output;

// Time [ms]
double dt, last_time, now;

// History
const int n_history = 5;                                  // how many data points (inclusing present to store)
int index = 0;                                            // index of present, incremented by 1 each loop
float theta_history[n_history] = { 10, 10, 10, 10, 10 };  // previous values of theta
float d_theta_history[n_history] = { 0, 0, 0, 0, 0 };     // previous values of d(theta)/dt
float torque_history[n_history] = { 0, 0, 0, 0, 0 };      // previous value of torque at the motor [N m]
float time_history[n_history] = { 0, 0, 0, 0, 0 };        // time elapsed at time of datapoint
float sample_time = 1.0;                                  // most recent time interval [ms]

// Motor command
int motor_pwm;
bool flash = false; // for flashing during TRACKING MODE 2

//==================================== CONSTANTS ==================================//
#define MUX_ADDR 0x70
//Light sensing
const float R = 1 / 2.1;                 // value depends on gain, lux response over rgb response
const float beta = 45 * PI / 180;        // lux sensor mounting angle in radians
const double denom = R * 2 * sin(beta);  // here so it only need to be run once
const double n = 1.088025599603545;      // fitted cos^n for rgb sensor
const int cutoff = 50;                   // minimum sensor value before the realtive angle is assumer >90 deg
// Cutoff value subject to further discretion

// Kalman const - ratio of model prediction to sensor data
const float K_kalman = 0;
// Gyro const - ratio of gyro data to light data
const float K_gyro = 0;  // change later

// PD constants
const float Kp = 1;
const float Kd = 1;

const float T0 = 0.018 * 9.81;  // Stall torque[Nm]
const int stall_rpm = 251;      // revolutions per minute


//==================================== SETUP ==================================//
/*float max_torque(float omega) {
  // Returns the maximum torque available (at full duty cycle)
  // as a function of the current motor rpm
  return T0 * (1 - omega / (stall_rpm * 2 * PI / 60));
}
*/

float five_bdif(float values[5], float times[5]) {
  // Computes a five-point backwards difference
  float sample_time = times[index] - times[(index - 1) % 5];  // based on the most recent time interval
  return (3 * values[(index - 4) % 5] - 16 * values[(index - 3) % 5] + 36 * values[(index - 2) % 5] - 48 * values[(index - 1) % 5] + 25 * values[index]) / (12 * sample_time);
}

float four_bdif(float values[4], float times[4]) {
  // Computes a five-point backwards difference
  float sample_time = times[index] - times[(index - 1) % 4];  // based on the most recent time interval
  return (11 * values[index] - 18 * values[(index - 1) % 4] + 9 * values[(index - 2) % 4] - 2 * values[(index - 3) % 4]) / (6 * sample_time);
}

void selectChannel(uint8_t i) {
  // Select Multiplex channel to connect to
  if (i > 3) return;
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setup() {
  Wire.begin();
  Wire.setWireTimeout(1000, true);
  Serial.begin(115200);
  Serial.println("Setup begin");
  //pinModes
  pinMode(logic, OUTPUT);
  pinMode(RP, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(T1, INPUT);
  pinMode(T2, INPUT);
  pinMode(T3, INPUT);

  // ===== Initialise motor off =====
  motor_pwm = 0;
  //Initialise pins
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, motor_pwm);
  digitalWrite(logic, HIGH);

  // ====== I2C initialisation ======
  Serial.println("MUX");
  Serial.println("Passed wire");
  // ========== Wake gyro ===========
  selectChannel(0);
  icm.begin_I2C();
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);    //Sets max acceleration rate measureable
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);  // Sets max rotation rate measureable

  // ===== Light sensor set up ======
  Serial.println("Lux");
  selectChannel(1);
  // Initialise LUX
  if (!veml.begin()) {
    Serial.println("VEML7700 not found on Port 1");
  } else {
    Serial.println("Lux on port 1 found");
  }

  selectChannel(3);
  if (!veml.begin()) {
    Serial.println("VEML7700 not found on Port 3");
  } else {
    Serial.println("Lux on port 3 found");
  }

  // Initialise RGB
  selectChannel(2);
  if (!tcs.begin()) {
    Serial.println("RGB sensor not found on Port 2");
  } else {
    Serial.println("RGB sensor found on Port 2");
  }

  Serial.println("Setup complete, starting countdown...");
  delay(1000);  // let it turn on
  Serial.println("Countdown complete");
}

//======================================================= MAIN LOOP =====================================================

void loop() {
  delay(1000);
  // =============================================== SELECT OPERATIONAL MODE ============================================
  RP_val = pulseIn(RP, HIGH, 30000);  //read the signal high width to determine operational mode
  RP_val = 900;
  if (RP_val < 990) {  // Get clarity on this logic query
    //===============================================GET DATA FROM ALL SENSORS ==========================================
    Serial.println("In loop");
    // Read Gyro
    selectChannel(0);  // 9 DoF IMU sensor
    icm.getEvent(&accel, &gyro, &Temp);
    gyro_z = gyro.gyro.z;
    gyro_z = gyro_z * -1;  // it does think clockwise is positive - needs to be flipped for transmition -fine for pd (une - gain)
    Serial.print("Gyro : ");
    Serial.println(gyro_z);

    // Send Gyro data
    // process gyro_z to byte array
    byte Gyro_array[6];
    for (int i = 0; i < 4; i++) {
      Gyro_array[i] = ((byte*)(&gyro_z))[i];
    }
    Gyro_array[4] = 1;  // byte ID for slave side ID 01
    Gyro_array[5] = 0;
    //send gyro_z to slave
    Wire.beginTransmission(SLAD);
    Wire.write(Gyro_array, 6);
    Wire.endTransmission();

    // Read left lux sensor
    selectChannel(1);
    VL = veml.readLux();
    Serial.print("Lux 1 : ");
    Serial.println(VL);
    // Read RGB sensor
    selectChannel(2);
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);
    // for the calculate lux function, if one of the values is zero (typically green)
    // the normalisation does a div 0 and Vrgb goes to its maximum.
    // Check if g is zero and add 1
    if (g == 0) { g = 1; }
    Vrgb = tcs.calculateLux(r, g, b);  // RGB sensor
    Serial.print("Vrgb : ");
    Serial.println(Vrgb);

    // Read right lux sensor
    selectChannel(3);
    VR = veml.readLux();
    Serial.print("Lux 3 : ");
    Serial.println(VR);
    //======================================================= SEARCH/TRACK MODE LOGIC ===================================================
    // MODES 1 AND 2  ==> Light outside of theta calculation range: PD control unavailable,
    // get light within FOV (+-45 deg)
    // MODE 3         ==> Light within FOV, control using PD
    // MODE 4         ==> Light locked on to, adjust for small pertubations using lux values, not theta - not sure about this
    // ===== MODE ONE =====
    // Light outside of both sensor's FOV
    if (VL < cutoff && VR < cutoff) {
      Serial.println("MODE ONE");
      if (VL < VR) {
        //need to move clockwise -
        theta_sen = 10.0;  // outside of possible range - indicates theta unknown
        theta = 10;
        motor_pwm = 180;          // 70% Power
        digitalWrite(IN1, HIGH);  // Sets motor polarity
        digitalWrite(IN2, LOW);   // Motor spins left -> CubeSat turns right (clockwise)
      } else {
        // VR < VL OR VL == VR
        //turn anticlockwise
        theta_sen = 10.0;
        theta = 10;
        motor_pwm = 180;
        digitalWrite(IN1, LOW);  // Motor spins right -> CubeSat turns left (counter-clockwise)
        digitalWrite(IN2, HIGH);
      }
    }

    // ===== MODE TWO =====
    // Light is in one lux sensor's FOV
    // only one sensor in cut off region - run at lower speed
    else if (not(VR > cutoff && VL > cutoff)) {  // One sensor below cutoff
      // Flash LED
      digitalWrite(LED, flash);
      flash = not flash;
      Serial.println("MODE TWO");
      if (VL > VR) {  // Light is in left FOV
        // Turn counter-clockwise
        theta_sen = 10;
        theta = 10;
        motor_pwm = 100;
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
      } else {  // Light is in right FOV
        // Turn clockwise
        theta_sen = 10;
        theta = 10;
        motor_pwm = 100;
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
      }
    }

    // ===== MODE THREE =====
    // Light within both sensor's FOV
    // Theta can be found, distance neglected
    // PD control
    else {
      digitalWrite(LED, HIGH);
      Serial.println("MODE THREE");
      // ============================= CALCULATE THETA ===============================//
      // USING LIGHT SENSING
      if (Vrgb == 0.0) { Vrgb = 0.1; }
      theta_sen = atan((VL - VR) / (denom * Vrgb));  // value for theta in radians
      Serial.print("theta_sen : ");
      Serial.println(theta_sen);
      // TIME LOOP RESET
      dt = millis() - last_time;
      last_time = millis();

      // INTEGRATING GYRO
      if (dt < 500) {
        theta_inertia = theta + gyro_z * dt;  // adds d(theta)/dt = gyro_z to previous value for theta

        // === BALANCE LIGHT SENSING WITH INERTIAL SENSING ===
        theta = (theta_inertia * K_gyro + theta_sen) / (1 + K_gyro);
      } else {
        theta = theta_sen;  // no filtering for first theta value in 500 ms
      }

      // =================================== PROCESS THETA ===========================//

      // PASS THROUGH KARMAN FILTER
      // Check if light has been in FOV for long enough
      Kalman_valid = true;
      for (int i = 0; i < n_history; i = i + 1) {
        if (theta_history[i] == 10 || theta_history[i] == 10.0) {
          Kalman_valid = false;
        }
      }
      if (Kalman_valid) {
        //can use mathmatical model here
      }

      // Update history with present
      theta_history[index] = theta;
      time_history[index] = now;
      d_theta = five_bdif(theta_history, time_history);      

      // MOTOR OUTPUT SET BY PD CONTROL
      error = theta;     //for transmition and formality
      P = Kp * error;  // change to theta if the model/ sensor smoothing is done

      // Check if theta data is recent enough to use
      if ((time_history[(index - 4) % 5] - time_history[index]) < 5 * dt) { // if oldest data point is within 5 time intervals
        d_theta = five_bdif(theta_history, time_history); // use five point backwards difference to differentiate theta
      }
      else {
        d_theta = gyro_z; // approximate d_theta using gyro data
      }

      D = Kd * gyro_z;
      pd_output = P + D;
      error_last = theta;

      index = (index + 1) % 5; // move current index along

      // Transmit error
      // process error to byte array
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
      //Wrap the pd_output to motor command, direction set by lux sensors -kind of?
      motor_pwm = fabs(pd_output * 31.875);  //write a wrap line max expected/ 255
      // Clockwise OR Counter-clockwise
      if (pd_output < 0) {  // Turn clockwise
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
      } else {  // Turn counter-clockwise
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
      }
    }

    // Send theta
    // process theta_sen to byte array
    byte theta_sen_array[6];
    for (int i = 0; i < 4; i++) {
      theta_sen_array[i] = ((byte*)(&theta_sen))[i];
    }
    //data ID 00 for light sensing theta
    theta_sen_array[4] = 0;
    theta_sen_array[5] = 0;
    //send theta_sen to slave
    Wire.beginTransmission(SLAD);
    Wire.write(theta_sen_array, 6);
    Wire.endTransmission();

    // ============= OUTPUT ============
    // Write PWM to motor pin
    analogWrite(ENA, motor_pwm);

    // Send Motor command
    byte motor_pwm_array[6];
    for (int i = 0; i < 4; i++) {
      motor_pwm_array[i] = ((byte*)(&motor_pwm))[i];
    }
    motor_pwm_array[4] = 1;
    motor_pwm_array[5] = 1;
    //send error to slave
    Wire.beginTransmission(SLAD);
    Wire.write(motor_pwm_array, 6);
    Wire.endTransmission();
    //
    // Joe - "This is cool af"
  }
  //============================================================================//
  else if (RP_val > 1400 && RP_val < 1600) {
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
  Serial.print("Theta : ");
  Serial.println(theta * 180 / PI);
}
