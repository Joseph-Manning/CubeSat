
#include <Wire.h>

#define SLAD 9
const int handshake = 10;

const char dataString[] = "abc,cum,cum,cum";

void setup() {
  Wire.begin();
  Serial.begin(9600);
  pinMode(handshake, INPUT);
}

void loop() {
  if (digitalRead(handshake)) {
    Wire.beginTransmission(SLAD);
    Wire.write((const uint8_t*)dataString, strlen(dataString));
    Wire.endTransmission();

    Serial.println(dataString);
    delay(30);   // ≈20 Hz
  }
}
