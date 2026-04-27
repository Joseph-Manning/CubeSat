/*Final script to flash to arduino A
This is the slave on the I2C bus
Author Kieran Orr
Contributors
Seb
Joe
Finlay*/

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
const int chipSelect = 10;

//======================================================================//
// FIFO BUFFER DEFINITIONS
#define PACKET_SIZE 6
#define BUFFER_SIZE 40   // large enough to absorb bursts

volatile byte packetBuffer[BUFFER_SIZE][PACKET_SIZE];
volatile int writeIndex = 0;
volatile int readIndex = 0;
volatile int packetCount = 0;

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

// SD file
File dataFile;
unsigned long lastFlush = 0;

//======================================================================//
// ISR: store each full packet into FIFO
void receiveEvent(int len) {
  if (len != PACKET_SIZE) {
    while (Wire.available()) Wire.read();
    return;
  }

  // Copy packet into FIFO slot
  for (int i = 0; i < PACKET_SIZE; i++) {
    packetBuffer[writeIndex][i] = Wire.read();
  }

  // Advance write index
  writeIndex = (writeIndex + 1) % BUFFER_SIZE;

  // Handle overflow (drop oldest)
  if (packetCount < BUFFER_SIZE) {
    packetCount++;
  } else {
    readIndex = (readIndex + 1) % BUFFER_SIZE;
  }
}

//======================================================================//
// Pop next packet from FIFO (non-blocking)
bool getNextPacket(byte *out) {
  noInterrupts();
  if (packetCount == 0) {
    interrupts();
    return false;
  }

  // Copy packet out
  for (int i = 0; i < PACKET_SIZE; i++) {
    out[i] = packetBuffer[readIndex][i];
  }

  // Advance read index
  readIndex = (readIndex + 1) % BUFFER_SIZE;
  packetCount--;
  interrupts();
  return true;
}

//======================================================================//
void setup() {
  pinMode(RP, INPUT);

  // I2C
  Wire.begin(SLAD);
  Wire.onReceive(receiveEvent);

  // SD
  SD.begin(chipSelect);
  dataFile = SD.open("datalog.txt", FILE_WRITE);
  dataFile.println("Time [ms], Theta [rad], Omega [rad/s], Error [rad], Command [PWM]");
}

//======================================================================//
void loop() {
  byte packet[PACKET_SIZE];

  // Process all available packets
  while (getNextPacket(packet)) {

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

  // Log only when all four values received
  if (gyroReady && thetaReady && errorReady && motorReady) {

    unsigned long t = millis();
    String s = "";

    s += t;
    s += ",";

    if (theta_sen == 10) {
      s += "Searching,";
      s += gyro_z;
      s += ",Unknown,";
    } else {
      s += theta_sen;
      s += ",";
      s += gyro_z;
      s += ",";
      s += error;
      s += ",";
    }

    s += motor_pwm;
    s += ",";

    dataFile.println(s);

    // Reset flags
    gyroReady = thetaReady = errorReady = motorReady = false;
  }

  // Flush SD every 200 ms
  if (millis() - lastFlush > 200) {
    dataFile.flush();
    lastFlush = millis();
  }
}
