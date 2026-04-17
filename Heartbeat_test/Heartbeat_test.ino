#include <MAVLink.h>
#include <SoftwareSerial.h>

//3.3 and 5 V pins
const int stepd = 2;
const int stepu = 3;
SoftwareSerial pixSerial(10,11); //RX,TX

void setup() {
  Serial.begin(57600);
  pixSerial.begin(57600); //if this is unstable go to 19200 range
  pinMode(stepu, OUTPUT);
  pinMode(stepd, OUTPUT);
  //const voltage pins
  digitalWrite(stepd, HIGH);
  digitalWrite(stepu, HIGH);
}

void loop() {

}