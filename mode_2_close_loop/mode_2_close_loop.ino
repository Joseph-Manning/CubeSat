

/*
  This is the closed loop code for mode 2 of CANSAT

*/
//Libraries===========================================================================
//Data logging:
#include <SPI.h>
#include <SD.h>
//Acceleration and gyro data:
#include <Wire.h>
//#include <math.h> //might be useful if there is space
#include <TCA9548A.h>
#include <Adafruit_ICM20948.h>
//Naming=============================================================================
TCA9548A mux; //multiplexer
Adafruit_ICM20948 icm; //9 DOF sensor
//pins
const int light_A = 7;
const int light_B = 6;
const int pulse = 9;
const int check_read = 3;
const int chipSelect = 10;
//ICM structure======================================================================
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp; //remember you need this or the machine spirit will be angered.
//SD=================================================================================
File dataFile;

//Global=============================================================================

double dt, last_time, now;
//Functions==========================================================================
//Kalman filter function gryo
double kalmanG(double theta_model, double theta_gyro, double desired_value) {
  float K_kalman = 1;
  double theta_measured = theta_model * K_kalman + (1 - K_kalman) * theta_gyro; 
  double error = desired_value - theta_measured;
  return error;
}
double kalmanA(double x_model, double x_acc, double desired_value){
  float K_kalman = 0;
  double x_measured = x_model * K_kalman + (1-K_kalman) * x_acc;
  double error = desired_value - x_measured;
  return error;
}
//pd
double pd(double error, double error_last, double dt) {
  float Kp = 1;
  float Kd = 1;
  double proportional = Kp * error;
  double derivative = Kd * ( error - error_last ) / dt;
  double output = proportional + derivative;
  return output;
}
//mathematical modelling
double accelleration_curve(double dt,double dx_i){ // predicts the distance crossed in the elapsed time
  //bool is_accel thinking of how to accond for deacceleration
  float a = 0.011592; //ms-2 predicted from thrust vs mass
  //float distance = 2; //m the thing its going up against
  double u = a*dt; //ms-1/calcs velocity
  double dx =  a * dt * dt + u * dt; //m quadratic assumption
  double sum_dx = dx_i - dx;
  double accel[2] = {u,sum_dx};
  return accel[2];//this array contains the velocity and the distance travelled
}
double gyro_curve(double dt){ // lienar model of roation fo elapsed time
  float m_r_r = 0.05; //rads-1 max rotation rate need to check 
  //float rads = 180  //rad
  double theta = m_r_r * dt; // rad
  return theta;
}
//===================================================================================
void setup() {
  // put your setup code here, to run once:
  Serial.begin(1200);
  //multiplexer setup
  Wire.begin();
  mux.begin();
  //gyro and accel setup
  mux.openChannel(3);
  icm.begin_I2C();
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G); //sets max accel range available
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS); //sets max rotation rate available
  //SD setup
  SD.begin();
  pinMode(check_read, OUTPUT);
  digitalWrite(check_read, LOW);
}

void loop() {
  // put your main code here, to run repeatedly:
  dataFile = SD.open("datalog_2.txt", FILE_WRITE);
  double now = millis();
  double dt = now - last_time;
  double last_time = now;
  /*
  Serial.print("  t[i]:"); Serial.print(now);
  Serial.print("  t[i-1]:"); Serial.print(last_time);
  Serial.println(" ");
  */
  //section below attaches shit to a datafile
  dataFile.print("  t[i]:"); dataFile.print(now);
  dataFile.print("  t[i-1]:"); dataFile.print(last_time);
  dataFile.println(" ");
  dataFile.close();
  
}
