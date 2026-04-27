/* Final script to flash to Arduino A
   Slave on the I2C bus
   Author: Kieran Orr
   Contributors: Seb, Finlay
*/
//======================================================================//
// Libraries
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

//======================================================================//
// I2C address
#define SLAD 9

//======================================================================//
// Pins
const int RP = 9;
const int T4 = 3;
const int H1 = 5;
const int H2 = 6;

const int chipSelect = 10;

//======================================================================//
// Data stores
volatile byte array[7];
volatile bool dataReady = false;

volatile bool gyroReady = false;
volatile bool thetaReady = false;
volatile bool errorReady = false;
volatile bool motorReady = false;

unsigned long RP_val;

float gyro_z;
float theta_sen;
float error;
float motor_pwm;

File dataFile;

//======================================================================//
// I2C receive ISR
void receiveEvent(int len) {
  if (len == 6) {
    for (int i = 0; i < 6; i++) {
      array[i] = Wire.read();
    }
    dataReady = true;
  } else {
    while (Wire.available()) Wire.read();
  }
}

//======================================================================//
void setup() {
  pinMode(RP, INPUT);

  // --- SD FIRST (NO I2C ACTIVE) ---
  SD.begin(chipSelect);
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  dataFile.println("Time [ms], Theta [rad], Omega [rad/s], Error [rad], Command [PWM]");

  // --- NOW enable I2C (SAFE) ---
  Wire.begin(SLAD);
  Wire.onReceive(receiveEvent);
}

//======================================================================//
void loop() {

  RP_val = pulseIn(RP, HIGH, 30000);

  if (RP_val < 990) {

    if (dataReady) {
      noInterrupts();   // protect array during decode
      dataReady = false;

      if (array[4] == 0 && array[5] == 0) {
        memcpy(&theta_sen, array, 4);
        thetaReady = true;
      }
      if (array[4] == 1 && array[5] == 0) {
        memcpy(&gyro_z, array, 4);
        gyroReady = true;
      }
      if (array[4] == 0 && array[5] == 1) {
        memcpy(&error, array, 4);
        errorReady = true;
      }
      if (array[4] == 1 && array[5] == 1) {
        memcpy(&motor_pwm, array, 4);
        motorReady = true;
      }

      interrupts();
    }

    if (gyroReady && thetaReady && errorReady && motorReady) {

      String dataString = "";
      unsigned long t = millis();

      dataString += t;
      dataString += ",";

      if (theta_sen == 10) {
        dataString += "Searching,";
        dataString += String(gyro_z);
        dataString += ",";
        dataString += "Unknown,";
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

      // --- SAFE SD WRITE (FILE KEPT OPEN) ---
      noInterrupts();
      dataFile.println(dataString);
      dataFile.flush();
      interrupts();

      gyroReady = thetaReady = errorReady = motorReady = false;
    }
  }
}
