#include <Servo.h>

Servo esc;
const int fan_pin = 7;

void setup() {
  esc.attach(fan_pin);
  esc.writeMicroseconds(1000); // Send arming signal
  delay(5000);
  
  //0 to mid speed
  for(int speed = 1000; speed <= 1500; speed += 10) {
    esc.writeMicroseconds(speed);
    delay(20);
  }
}

void loop() {
  esc.writeMicroseconds(1500); 
}
