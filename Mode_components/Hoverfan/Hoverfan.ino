/*
works on the concept of a push button to stay on
digitalwrite so save on PWM pins since it will be a constant thrust percentage so we could tweak it w/ resitors ect (if wrong it is an easy fix)
*/
const int signal_pin = 6; 
const int thrust_pin = 5; //example pins
bool ARMED = false;
void setup() {
  Serial.begin(9600);
  pinMode(signal_pin,INPUT);
  pinMode(thrust_pin,OUTPUT);
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
 float signal_read = digitalRead(6);
 Serial.println(signal_read);//is the thing running NO, and has signal switched been recieved
 if(signal_read==HIGH){
  if
  digitalWrite(thrust_pin,HIGH);
 } else{
  digitalWrite(thrust_pin,LOW);
 }
}
