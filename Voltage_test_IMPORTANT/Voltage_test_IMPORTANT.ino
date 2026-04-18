/*VOLTAGE TEST
MUST COMPLETE BEFORE CONNECTING TO PIX 32
OR YOU WILL DAMAGE IT BEYOND REPAIR*/

/*INSTRUCTIONS
1: flash a blank sketch to arduino
2: follow the pin mapping below
3: flash this script to arduino
4: connect the multimeter gnd to the test gnd jumper lead
5: with the live probe test the voltage readings in each GH1 connection port
6: RX, TX, RTS, CTS must be ~3.3V, gnd must be gnd
7: if above condition met, remove power from arduino and pix
8: connect the connector to the required Telem port
9: IMPORTANT now ensure that no wires have come loose, if they have they must be secured in port
10: arduino and pix now safely connected - cross your fingers*/


//pins - to be connected to arduino
const int V3 = 2; //should be within 1% of 3.3V - live to board
const int V5 = 8; //live wire directly to step module
const int RX = 5; //blue wire
const int TX = 6; //green wire
const int RTS = 10; //white wire
const int CTS = 11; //yellow wire 
//ensure ground wire is secure
//add an additional gnd jumper cable to an arduino gnd pin (they all share Vcc ref)

void setup() {
  //pin modes
  pinMode(V3, OUTPUT);
  pinMode(V5, OUTPUT);
  pinMode(RX, OUTPUT);
  pinMode(TX, OUTPUT);
  pinMode(RTS, OUTPUT);
  pinMode(CTS, OUTPUT);
  //set 5V output
  digitalWrite(V3, HIGH);
  digitalWrite(V5, HIGH);
  digitalWrite(RX, HIGH);
  digitalWrite(TX, HIGH);
  digitalWrite(RTS, HIGH);
  digitalWrite(CTS, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:

}
