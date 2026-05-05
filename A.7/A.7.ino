/*Final script to flash to arduino A
This is the master on the I2C bus
Authors Kieran, Joe
Contributors
Seb
Finley*/
//======================================================================//
/*Pin map
3 To PIX GPIO 7
4 LOGIC to motor driver
5 To motor driver ENA
6 To motor driver IN1
7 To motor driver IN2
8 To LED
9 reaction wheel signal line from pix

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
const int RCRW = 9;  //for RC RW

//motor control
const int ENA = 5;
const int IN1 = 6;  //only need to be H/L
const int IN2 = 7;  //only need to be H/L

//LED pin
const int LED = 8;
const int handshake = 10;

//================================ DATA STRUCTURES ==============================//
//Data struct
//ICM struc
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t Temp;

char dataString[64];
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
float K_gyro_loop;
//kalman filter/PD
float error;
float error_last;
bool Kalman_valid;
float d_theta;

float p;
float d;
float pd_output;
float theta_last;
float time_last;

// Time [ms]
float dt;
unsigned long time, last_time, now;
// History
const int n_history = 5;                                  // how many data points (inclusing present to store)
int index = 0;                                            // index of present, incremented by 1 each loop
float theta_history[n_history] = { 10, 10, 10, 10, 10 };  // previous values of theta
float d_theta_history[n_history] = { 0, 0, 0, 0, 0 };     // previous values of d(theta)/dt
float torque_history[n_history] = { 0, 0, 0, 0, 0 };      // previous value of torque at the motor [N m]
float error_history[n_history] = { 0, 0, 0, 0, 0 };
float time_history[n_history] = { 0, 0, 0, 0, 0 };  // time elapsed at time of datapoint
float sample_time = 50.0;                           // most recent time interval [ms]

//mode 2 section, no time for organisation
float gyro_z_history[10] = {};
int index_count = 0;
float smoothed_gyro_z = 0;
float yaw_pos = 0;
float yaw_error = 0;
float gyro_error = 0;
//reaction wheel control
int RW_IN;
int RW_PWM;

// Motor command
int motor_pwm;
bool flash = false;  // for flashing during TRACKING MODE 2


//==================================== CONSTANTS ==================================//
#define MUX_ADDR 0x70
//Light sensing
const float R = 1 / 2.1;                 // value depends on gain, lux response over rgb response
const float beta = 45 * PI / 180;        // lux sensor mounting angle in radians
const double denom = R * 2 * sin(beta);  // here so it only need to be run once
const double n = 1.088025599603545;      // fitted cos^n for rgb sensor
const int cutoff = 300;                  // minimum sensor value before the realtive angle is assumer >90 deg
// Cutoff value subject to further discretion

// Kalman const - ratio of model prediction to sensor data
const float K_kalman = 0;
// Gyro const - ratio of gyro data to light data
const float K_gyro = 0;  // change later

// PD constants
const float Kp = 50;
const float Kd = 30;

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
  float DT = (times[index] - times[(index - 1) % 5]) / 1000;  // based on the most recent time interval
  Serial.print("Sample_time:");
  Serial.print(DT);
  Serial.print(",");
  return (3 * values[(index - 4) % 5] - 16 * values[(index - 3) % 5] + 36 * values[(index - 2) % 5] - 48 * values[(index - 1) % 5] + 25 * values[index]) / (12 * DT);
}

float four_bdif(float values[4], float times[4]) {
  // Computes a five-point backwards difference
  float sample_time = (times[index] - times[(index - 1) % 4]) / 1000;  // based on the most recent time interval
  return (11 * values[index] - 18 * values[(index - 1) % 4] + 9 * values[(index - 2) % 4] - 2 * values[(index - 3) % 4]) / (6 * sample_time);
}

float rolling_avg(float values[5]) {
  float s = 0;
  for (int i = 0; i < 5; i++) {
    s += values[i];
  }
  return s / 5;
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
  pinMode(RCRW, INPUT);
  pinMode(RP, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(handshake, INPUT);



  // ===== Initialise motor off =====
  motor_pwm = 0;
  //Initialise pins
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, motor_pwm);
  digitalWrite(logic, HIGH);

  // ====== I2C initialisation ======
  //Serial.println("MUX");
  //Serial.println("Passed wire");
  // ========== Wake gyro ===========
  selectChannel(0);
  icm.begin_I2C();
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);    //Sets max acceleration rate measureable
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);  // Sets max rotation rate measureable

  // ===== Light sensor set up ======
  //Serial.println("Lux");
  selectChannel(1);
  // Initialise LUX
  if (!veml.begin()) {
    //Serial.println("VEML7700 not found on Port 1");
  } else {
    //Serial.println("Lux on port 1 found");
  }

  selectChannel(3);
  if (!veml.begin()) {
    //Serial.println("VEML7700 not found on Port 3");
  } else {
    //Serial.println("Lux on port 3 found");
  }

  // Initialise RGB
  selectChannel(2);
  if (!tcs.begin()) {
    //Serial.println("RGB sensor not found on Port 2");
  } else {
    //Serial.println("RGB sensor found on Port 2");
  }

  //Serial.println("Setup complete, starting countdown...");
  //delay(10000);  // let it turn on
  //Serial.println("Countdown complete");
}

//======================================================= MAIN LOOP =====================================================

void loop() {
  delay(10);
  // =============================================== SELECT OPERATIONAL MODE ============================================
  RP_val = pulseIn(RP, HIGH, 30000);  //read the signal high width to determine operational mode
  if (RP_val < 990) {                 // Get clarity on this logic query
    //===============================================GET DATA FROM ALL SENSORS ==========================================
    // Read Gyro
    selectChannel(0);  // 9 DoF IMU sensor
    icm.getEvent(&accel, &gyro, &Temp);
    gyro_z = gyro.gyro.z;
    gyro_z = gyro_z * -1;
    //    Serial.print("Gyro:");
    //    Serial.print(gyro_z);
    //    Serial.print(",y:");
    //    Serial.print(gyro.gyro.y);
    //    Serial.print(",x:");
    //    Serial.print(gyro.gyro.x);

    // GYRO UTTERLEY FUCKING USELESS
    // ABSOLOLUTELY NO RELATION TO ANGULAR SPEED OR ROTATION

    //if (fabs(gyro_z) > 0.5){gyro_z=0;}
    // it does think clockwise is positive - needs to be flipped for transmition -fine for pd (une - gain)

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
    Serial.print("Lux1:");
    Serial.print(VL / 1000);
    Serial.print(",");
    // Read RGB sensor
    selectChannel(2);
    uint16_t r, g, b, c;
    tcs.getRawData(&r, &g, &b, &c);
    // for the calculate lux function, if one of the values is zero (typically green)
    // the normalisation does a div 0 and Vrgb goes to its maximum.
    // Check if g is zero and add 1
    if (g == 0) { g = 1; }
    Vrgb = tcs.calculateLux(r, g, b);  // RGB sensor
    if (Vrgb < 1) { Vrgb = 10; }
    Serial.print("Vrgb:");
    Serial.print(Vrgb / 1000);
    Serial.print(",");

    // Read right lux sensor
    selectChannel(3);
    VR = veml.readLux();
    Serial.print("Lux3:");
    Serial.print(VR / 1000);
    Serial.print(",");
    //======================================================= SEARCH/TRACK MODE LOGIC ===================================================
    // MODES 1 AND 2  ==> Light outside of theta calculation range: PD control unavailable,
    // get light within FOV (+-45 deg)
    // MODE 3         ==> Light within FOV, control using PD
    // MODE 4         ==> Light locked on to, adjust for small pertubations using lux values, not theta - not sure about this

    // ===== MODE ONE =====
    // Light outside of both sensor's FOV
    if (VL < cutoff && VR < cutoff) {
      //Serial.println("MODE ONE");
      digitalWrite(LED, LOW);
      d = 0;
      if (VL > VR) {
        //need to move clockwise -
        theta_sen = 10.0;  // outside of possible range - indicates theta unknown
        theta = 10;
        p = 3 * PI / 2;
      } else {
        // VR < VL OR VL == VR
        //turn anticlockwise
        theta_sen = 10.0;
        theta = 10;
        p = -3 * PI / 2;
      }
    }

    // ===== MODE TWO =====
    // Light is in one lux sensor's FOV
    // only one sensor in cut off region - run at lower speed
    else if (not(VR > cutoff && VL > cutoff)) {  // One sensor below cutoff
      // Flash LED
      digitalWrite(LED, flash);
      flash = not flash;
      //Serial.println("MODE TWO");
      d = 0;
      if (VL > VR) {  // Light is in left FOV
        // Turn counter-clockwise
        theta_sen = 10;
        theta = 10;
        p = PI / 2;

      } else {  // Light is in right FOV
        // Turn clockwise
        theta_sen = 10;
        theta = 10;
        p = -PI / 2;
      }
    }

    // ===== MODE THREE =====
    // Light within both sensor's FOV
    // Theta can be found, distance neglected
    // PD control
    else {
      digitalWrite(LED, HIGH);
      //Serial.println("MODE THREE");
      // ============================= CALCULATE THETA ===============================//
      // USING LIGHT SENSING
      if (Vrgb == 0.0) { Vrgb = 0.1; }
      theta_sen = atan((VL - VR) / (denom * Vrgb));  // value for theta in radians
      Serial.print("theta_sen:");
      if (theta_sen == 10) {
        Serial.print(0);
      } else {
        Serial.print(theta_sen);
      }
      Serial.print(",");

      dt = millis() - time_last;
      time_last = millis();

      p = theta_sen;  // check sign
      d = (theta_sen - theta_last) / dt;
      theta_last = theta_sen;
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
    if (d > 3) { d = 3; }
    motor_pwm = Kp * p + Kd * d;  // if positive -> turn right

    if (motor_pwm > 255) { motor_pwm = 255; }
    if (motor_pwm < -255) { motor_pwm = -255; }

    if (motor_pwm > 0) {
      digitalWrite(IN1, HIGH);  // Sets motor polarity
      digitalWrite(IN2, LOW);   // Motor spins left -> CubeSat turns right (clockwise)
    } else {
      digitalWrite(IN1, LOW);  // Motor spins right -> CubeSat turns left (counter-clockwise)
      digitalWrite(IN2, HIGH);
    }

    // Write PWM to motor pin
    analogWrite(ENA, abs(motor_pwm));

    Serial.print("p:");
    Serial.print(p);
    Serial.print(",d:");
    Serial.print(d);
    Serial.print(",PWM:");
    Serial.print(motor_pwm);
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
  }
  //============================================================================//
  else if (RP_val > 1800) {
    motor_pwm = 0;
    RW_IN = pulseIn(RCRW, 1, 30000);  //read condition of pulse

    //call gyro data
    selectChannel(0);  // 9 DoF IMU sensor
    icm.getEvent(&accel, &gyro, &Temp);
    gyro_z = gyro.gyro.z;
    gyro_z = gyro_z * -1;

    //gyro filter
    gyro_z_history[index_count] = gyro_z;
    index_count++;
    if (index_count == 10) {
      index_count = 0;
    }
    float sum = 0;
    for (int i = 0; i < 10; i++) {
      sum += gyro_z_history[i];
    }

    smoothed_gyro_z = sum / 10;

    //manual command section
    if (RW_IN < 1452) {
      RW_IN = constrain(RW_IN, 982, 1452);
      digitalWrite(IN1, 0);
      digitalWrite(IN2, 1);
      int signal_in = 1452 - RW_IN;  // this wraps correctly
      RW_PWM = signal_in * 0.54;
      RW_PWM = round(RW_PWM);
      RW_PWM = constrain(RW_PWM, 100, 255);
      analogWrite(ENA, RW_PWM);
      Serial.println(RW_IN);
      Serial.println(RW_PWM);
    } else if (RW_IN > 1532) {
      RW_IN = constrain(RW_IN, 1532, 2002);
      digitalWrite(IN1, 1);
      digitalWrite(IN2, 0);
      int signal_in = RW_IN - 1532;
      RW_PWM = signal_in * 0.54;
      RW_PWM = round(RW_PWM);
      RW_PWM = constrain(RW_PWM, 100, 255);
      analogWrite(ENA, RW_PWM);
      Serial.println(RW_IN);
      Serial.println(RW_PWM);
    } else {
      //yaw stabilisation
      time = millis();  //in seconds
      dt = (time - time_last) / 1000;
      yaw_pos += smoothed_gyro_z * dt;
      yaw_pos *= 0.995;  // bleed
      yaw_pos = constrain(yaw_pos, -0.5, 0.5);
      time_last = time;
      yaw_error = -yaw_pos;
      gyro_error = -smoothed_gyro_z;
      float Kp_stab = 200;
      float Kd_stab = 100;
      RW_PWM = abs(Kp_stab * yaw_error + Kd_stab * gyro_error);
      if (yaw_error < 0) {  // need to push anticlockwise
        digitalWrite(IN1, 0);
        digitalWrite(IN2, 1);
      } else if (yaw_error > 0) {  //need to push clockwise
        digitalWrite(IN1, 1);
        digitalWrite(IN2, 0);
      }
      RW_PWM = constrain(RW_PWM, 0, 255);
      analogWrite(ENA, RW_PWM);
      Serial.println(RW_IN);
      Serial.println(RW_PWM);

      //prepare to send data
      dataString[0] = '\0';  // clear it before use

      float gyro_3dp = round(smoothed_gyro_z * 1000.0f) / 1000.0f;
      char gyro_str[12];
      dtostrf(gyro_3dp, 6, 3, gyro_str);

      float RW_PWM_3dp = (float)RW_PWM;
      char RW_PWM_str[12];
      dtostrf(RW_PWM_3dp, 6, 3, RW_PWM_str);

      //append
      strcat(dataString, gyro_str);
      strcat(dataString, ",");
      strcat(dataString, RW_PWM_str);
      strcat(dataString, ",");


      //now transmit
      if (digitalRead(handshake)) {
        Wire.beginTransmission(SLAD);
        Wire.write((const uint8_t*)dataString, strlen(dataString));
        Wire.endTransmission();

        Serial.println(dataString);
        delay(30);
      }
    }
  } else {
    //leave for now
  }
}
