/*Final script to flash to arduino A
This is the slave on the I2C bus
Author Kieran Orr
Contributors
Seb
Finlay*/
//======================================================================//
//Libraries
#include <Wire.h>  //included for I2C communication

//Data logging
#include <SPI.h>
#include <SD.h>
//======================================================================//
//Define address of slave Arduino
#define SLAD 9
//======================================================================//
/*Pin map
10 Chip select on SD shield
11 MOSI on SD shield
12 MISO on SD shield
13 SCK on SD shield
3.3V on SD shield
GND spare GND port

A4 Data pin (blue I2C connect)
A5 Clock pin (yellow I2C connect)
GND shared with master GND*/
//======================================================================//
//pins
//Mode ID pin
const int RP = 9;
//Fan pins
const int T4 = 3;
const int H1 = 5;
const int H2 = 6;

//SD
const int chipSelect = 10;
//======================================================================//
//define data stores
//assemble on slave side
volatile byte array[7];

//ready to read from receive event
volatile bool dataReady;

//mode 1 data check update
volatile bool gyroReady;
volatile bool thetaReady;
volatile bool errorReady;
volatile bool motorReady;

//mode id
unsigned long RP_val;

//SD
File dataFile;
//======================================================================//
//define global variables
//data to save
float gyro_z;
float theta_sen;
float error;
float motor_pwm;

int counter = 1;  // save every 20 cycles
//======================================================================//
//define function for data recieved
void receiveEvent(int check_len) {
  for (int i = 0; i < check_len; i++) {
    array[i] = Wire.read();  //general buffer array
  }
  dataReady = 1;
}
//======================================================================//
void setup() {
  //mode pin
  pinMode(RP, INPUT);

  //prepare data bools
  dataReady = 0;
  gyroReady = 0;
  thetaReady = 0;
  errorReady = 0;
  motorReady = 0;

  //initialise I2C
  Wire.begin(SLAD);
  Wire.onReceive(receiveEvent);

  //SD
  SD.begin(chipSelect);
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  dataFile.println("Time [ms], Theta [rad], Omega [rad/s], Error [rad], Command [PWM]");
}

void loop() {
  RP_val = pulseIn(RP, HIGH, 30000);
  if (RP_val < 990) {
    if (dataReady) {
      dataReady = false;
      if (array[4] == 0 && array[5] == 0) {
        memcpy(&theta_sen, array, 4);
        thetaReady = 1;
      }
      if (array[4] == 1 && array[5] == 0) {
        memcpy(&gyro_z, array, 4);
        gyroReady = 1;
      }
      if (array[4] == 0 && array[5] == 1) {
        memcpy(&error, array, 4);
        errorReady = 1;
      }
      if (array[4] == 1 && array[5] == 1) {
        memcpy(&motor_pwm, array, 4);
        motorReady = 1;
      }
    }
    if (gyroReady == 1 && thetaReady == 1 && errorReady == 1 && motorReady == 1) {
      //data logging
      // make a string for assembling the data to log:
      String dataString = "";

      //write values to the string:
      unsigned long t = millis();
      dataString += t;
      dataString += ",";
      if (theta_sen == 10) {
        dataString += "Searching";
        dataString += String(gyro_z);
        dataString += ",";
        dataString += "Unknown";
        dataString += ",";
      } else {
        dataString += String(theta_sen);
        dataString += ",";
        dataString += String(gyro_z);
        dataString += ",";
        dataString += String(error);
        dataString += ",";
      }
      dataString += String(motor_pwm);
      dataString += ",";
      //open the file
      //if (counter == 1) {File dataFile = SD.open("datalog.txt", FILE_WRITE);} // see if we can get away without declaring FIle everytime
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
      // if the file is available, write to it:
      if (dataFile) {
        dataFile.println(dataString);
        dataFile.close();
        //if (counter == 10) {dataFile.close(); counter = 0;}
      }
      //counter += 1;
      gyroReady = 0;
      thetaReady = 0;
      errorReady = 0;
      motorReady = 0;
    } else {
    }
  }
}
