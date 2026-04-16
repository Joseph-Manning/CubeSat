#include <MAVLink.h> //provides mavlink commands
#include <SoftwareSerial.h> //allows for serial monitor use

//pins
const int test = 3;
//software serial pins
SoftwareSerial pixSerial(10,11); //RX,TX

void setup() {
Serial.begin(9600);
pixSerial.begin(57600); //if this is unstable go to 20k range
pinMode(test,INPUT);
}

void loop() {
unsigned long read = pulseIn(test,HIGH);
if (read<900){
  Serial.write("no signal");
}
else {
  Serial.write("signal");
}
Serial.println();
delay(1000);

}
