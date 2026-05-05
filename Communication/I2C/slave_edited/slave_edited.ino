
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define SLAD 9
#define BUFFER_SIZE 32

const int chipSelect = 10;
const int handshake = 8;

volatile bool dataReady = false;
volatile uint8_t rxLen = 0;
char rxBuffer[BUFFER_SIZE + 1];

File dataFile;

/******** I2C ISR ********/
void receiveEvent(int count) {
  if (count > BUFFER_SIZE) count = BUFFER_SIZE;

  rxLen = 0;
  while (Wire.available() && rxLen < BUFFER_SIZE) {
    rxBuffer[rxLen++] = Wire.read();
  }
  rxBuffer[rxLen] = '\0';   // C-string safe
  dataReady = true;
}

void setup() {
  pinMode(handshake, OUTPUT);
  digitalWrite(handshake, HIGH);   // ready

  Serial.begin(9600);

  Wire.begin(SLAD);
  Wire.onReceive(receiveEvent);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD init failed!");
    while (1);
  }

  dataFile = SD.open("log.csv", FILE_WRITE);
  Serial.println("Slave ready");
}

void loop() {

  if (dataReady) {
    digitalWrite(handshake, LOW);   // stop master

    noInterrupts();
    char localCopy[BUFFER_SIZE + 1];
    strcpy(localCopy, rxBuffer);
    dataReady = false;
    interrupts();

    if (dataFile) {
      dataFile.println(localCopy);
      dataFile.flush();
      Serial.println(localCopy);
    }

    digitalWrite(handshake, HIGH);  // allow master
  }
}
