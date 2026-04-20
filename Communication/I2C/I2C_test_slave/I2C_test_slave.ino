#include <Wire.h>

#define SLAVE_ADD 9

#define ANSWERSIZE 5

String answer = "Hello";

void receiveEvent(){
  while(0 < Wire.available()) {
    byte x = Wire.read();
  }
  Serial.println("receive Event");
}

void requestEvent(){
  byte response[ANSWERSIZE];

  for (byte i = 0; i<ANSWERSIZE; i++) {
    response[i] = (byte)answer.charAt(i);
  }
  Wire.write(response,sizeof(response));

  Serial.println("Request Event");
}
void setup() {
  Wire.begin(SLAVE_ADD);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  Serial.begin(9600);
  Serial.println("slave side");
}

void loop() {
  delay(5000);
}
