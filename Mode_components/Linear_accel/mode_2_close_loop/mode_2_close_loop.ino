

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
TCA9548A mux;           //multiplexer
Adafruit_ICM20948 icm;  //9 DOF sensor
Adafruit_Sensor *icm_accel, *icm_gyro,*icm_lsk,*icm_temp;
//pins
const int light_A = 7;
const int light_B = 6;
const int pulse = 9;
const int check_read = 3;
const int chipSelect = 10;
const int pinout = 5;
//ICM structure======================================================================
sensors_event_t accel;
sensors_event_t gyro;
sensors_event_t temp; 
//SD=================================================================================
File dataFile;

//Global=============================================================================
double acceleration, last_acceleration, gyro_init, last_gyro, initial_velocity;
double last_error_acc, last_error_gyro, error_acc, error_gyro, dx_i;
float dt, last_time, now;

//Functions==========================================================================
//Kalman filter function gryo
double kalmanG(double theta_model, double theta_gyro, double desired_value) {
  float K_kalman = 1;
  double theta_measured = theta_model * K_kalman + (1 - K_kalman) * theta_gyro;
  double error = desired_value - theta_measured;
  return error;
}
double kalmanA(double x_model, double x_acc, double desired_value) {
  float K_kalman = 0;
  double x_measured = x_model * K_kalman + (1 - K_kalman) * x_acc;
  double error = desired_value - x_measured;
  return error;
}
//pd
double pid(double error, double error_last, double dt) {
  //returns xt
  float Kp = 1;
  float Ki = 1;
  float Kd = 1;  //ZN method?
  double proportional = Kp * error;
  //double intergral = Ki * dt * (error + error_last);  //I don't know if this is actual intergral but reading up on the intergral we need this.
  double derivative = Kd * (error - error_last) / dt;
  double output = proportional + intergral + derivative;
  //double output[2] = {xt,}
  return output; /*
  returns output which is the response of the throttle desired from the translational thruster
  */
}
//mathematical modelling
double acceleration_curve(float dt, double dx_i) {
  /* predicts the distance crossed in the elapsed time x_i is previous dx dt is elapsed time
  */
  //bool is_accel thinking of how to accond for deacceleration
  float a = 0.11592;  //ms-2 predicted from thrust vs mass
  //float distance = 2; //m the thing its going up against
  float dts = dt / 1000;
  double u = a * dts;                   //ms-1/calcs velocity
  double dx = a * dts * dts + u * dts;  //m quadratic assumption
  double sum_dx = dx_i + dx;
  return sum_dx;  //returns distance travelled in m (since time is in ms)
}
double gyro_curve(double dt) {  // lienar model of roation fo elapsed time
  float m_r_r = 0.05;           //rads-1 max rotation rate need to check
  //float rads = 180  //rad
  double theta = m_r_r * dt;  // rad
  return theta;
}
//===================================================================================
void setup() {
  //remember you need this or the machine spirit will be angered.
  // put your setup code here, to run once:
  Serial.begin(9600);
  //multiplexer setup
  Wire.begin();
  mux.begin();
  //gyro and accel setup
  mux.openChannel(3);
  icm.begin_I2C();
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);    //sets max accel range available
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);  //sets max rotation rate available
  //SD setup
  SD.begin();
  pinMode(check_read, OUTPUT);
  digitalWrite(check_read, LOW);

  //temp
  pinMode(light_A, OUTPUT);  //forward thruster
  pinMode(light_B, OUTPUT);  //reverse thruster

  ;
}

void loop() {

  // put your main code here, to run repeatedly:
  dataFile = SD.open("datalog_2.txt", FILE_WRITE);
  //Time
  now = millis();
  dt = now - last_time;
  last_time = now;
  //data from the thing
  icm_accel->getEvent(&accel);
  icm_gyro->getEvent(&gyro);
  //this calcs for the recorded acceleration:
  acceleration = accel.acceleration.x;
  double final_velocity = acceleration* dt;  //velocity from this cycle
  double x_acc = 0.5 * ( 0.5 * acceleration * dt* dt + final_velocity + 0.5 * last_acceleration * dt * dt + initial_velocity ) ;
  last_acceleration = acceleration;
  double intital_velocity = final_velocity;
  //this is the modelling section
  double x_model = acceleration_curve(dt, dx_i);
  //error calculation
  error_acc = kalmanA(x_model, x_acc, 0);
  //logic being that acncrv is the ideal model ff to change but this gives us error
  double output_acc = pid(error_acc, last_error_acc, dt);  //output between 0->225
  last_error_acc = error_acc;
  /*
  if(error>0.95){
    //forward thruster full
    //light a is forward thruster
    digitalWrite(light_A,HIGH);
    digitalWrite(light_B,LOW);
    
  }
  else if(error>0.01){
    //backward thruster full
    digitalWrite(light_A,HIGH);
    digitalWrite(light_B,LOW);
  }
  else{
    //ensure that we have reached end then spinning time :)

    
  }
  */
}