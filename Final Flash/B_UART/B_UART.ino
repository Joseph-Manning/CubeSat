/* Final UART Slave Script
   Replaces I2C with UART
   Author: Kieran Orr
   Contributors: Seb, Finlay
*/
//======================================================================//
// Libraries
#include <SPI.h>
#include <SD.h>

//======================================================================//
// Pins
const int RP = 9;
const int chipSelect = 10;

//======================================================================//
// UART packet format
// [4 bytes float][1 byte id1][1 byte id2]
const int PACKET_SIZE = 6;
byte packet[PACKET_SIZE];
int packetIndex = 0;
bool packetReady = false;

//======================================================================//
// Data flags
bool gyroReady = false;
bool thetaReady = false;
bool errorReady = false;
bool motorReady = false;

// Data storage
float gyro_z;
float theta_sen;
float error;
float motor_pwm;

unsigned long RP_val;

File dataFile;

//======================================================================//
// UART packet reader
void readUART() {
  while (Serial.available()) {
    packet[packetIndex++] = Serial.read();

    if (packetIndex == PACKET_SIZE) {
      packetReady = true;
      packetIndex = 0;
    }
  }
}

//======================================================================//
void setup() {
  pinMode(RP, INPUT);

  Serial.begin(115200);

  // --- SD FIRST ---
  SD.begin(chipSelect);
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  dataFile.println("Time [ms], Theta [rad], Omega [rad/s], Error [rad], Command [PWM]");
}

//======================================================================//
void loop() {

  RP_val = pulseIn(RP, HIGH, 30000);

  if (RP_val < 990) {

    // Read UART stream
    readUART();

    // If a full packet arrived, decode it
    if (packetReady) {
      packetReady = false;

      byte id1 = packet[4];
      byte id2 = packet[5];

      if (id1 == 0 && id2 == 0) {
        memcpy(&theta_sen, packet, 4);
        thetaReady = true;
      }
      else if (id1 == 1 && id2 == 0) {
        memcpy(&gyro_z, packet, 4);
        gyroReady = true;
      }
      else if (id1 == 0 && id2 == 1) {
        memcpy(&error, packet, 4);
        errorReady = true;
      }
      else if (id1 == 1 && id2 == 1) {
        memcpy(&motor_pwm, packet, 4);
        motorReady = true;
      }
    }

    // When all four values have arrived, log them
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

      // --- SAFE SD WRITE ---
      dataFile.println(dataString);
      dataFile.flush();

      gyroReady = thetaReady = errorReady = motorReady = false;
    }
  }
}
