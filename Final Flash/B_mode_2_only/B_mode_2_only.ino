//Libs
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#define SLAD 9
#define BUFFER_SIZE 64

//pins
//sd pins
const int chipSelect = 7;
const int handshake = 8;

//fan intercept pins
const int H1 = 11;
const int H2 = 10;
const int Backward_fan = 9;
const int Forward_fan = 6;
const int Left_fan = 5;
const int Right_fan = 3;

//data store for intercept
int H1_in;
int H2_in;
int Backward_fan_in;
int Forward_fan_in;
int Left_fan_in;
int Right_fan_in;


volatile bool dataReady = false;
volatile uint8_t rxLen = 0;
char rxBuffer[BUFFER_SIZE + 1];
char localCopy[BUFFER_SIZE + 1];

File dataFile;

/******** I2C ISR ********/
void receiveEvent(int count) {
  if (count > BUFFER_SIZE) count = BUFFER_SIZE;

  rxLen = 0;
  while (Wire.available() && rxLen < BUFFER_SIZE) {
    rxBuffer[rxLen++] = Wire.read();
  }
  rxBuffer[rxLen] = '\0';  // C-string safe
  dataReady = true;
}

void setup() {
  //fan read set up
  pinMode(H1, INPUT);
  pinMode(H2, INPUT);
  pinMode(Backward_fan, INPUT);
  pinMode(Forward_fan, INPUT);
  pinMode(Left_fan, INPUT);
  pinMode(Right_fan, INPUT);
  //cts set up
  pinMode(handshake, OUTPUT);
  digitalWrite(handshake, HIGH);  // ready

  Serial.begin(9600);

  Wire.begin(SLAD);
  Wire.onReceive(receiveEvent);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD init failed!");
    while (1)
      ;
  }

  dataFile = SD.open("log.csv", FILE_WRITE);
  dataFile.println("Omega_z [rad/s], Motor PWM, Hover 1, Hover 2, Backward_fan, Forward_fan, Left_fan, Right_fan, Time [ms]");
  Serial.println("Slave ready");
}

void loop() {

  if (dataReady) {
    digitalWrite(handshake, LOW);  // stop master

    noInterrupts();
    strcpy(localCopy, rxBuffer);
    dataReady = false;
    interrupts();
    //read fan input
    H1_in = pulseIn(H1, 1, 30000);
    H2_in = pulseIn(H2, 1, 30000);
    Backward_fan_in = pulseIn(Backward_fan, 1, 30000);
    Forward_fan_in = pulseIn(Forward_fan, 1, 30000);
    Left_fan_in = pulseIn(Left_fan, 1, 30000);
    Right_fan_in = pulseIn(Right_fan, 1, 30000);
    unsigned long time = millis();
    char time_str[12];
    ultoa(time, time_str, 10);


    //append local copy with required fan data
    char H1_str[12];
    itoa(H1_in, H1_str, 10);

    char H2_str[12];
    itoa(H2_in, H2_str, 10);

    char Backward_fan_str[12];
    itoa(Backward_fan_in, Backward_fan_str, 10);

    char Forward_fan_str[12];
    itoa(Forward_fan_in, Forward_fan_str, 10);

    char Left_fan_str[12];
    itoa(Left_fan_in, Left_fan_str, 10);

    char Right_fan_str[12];
    itoa(Right_fan_in, Right_fan_str, 10);

    //append
    strcat(localCopy, H1_str);
    strcat(localCopy, ",");

    strcat(localCopy, H2_str);
    strcat(localCopy, ",");

    strcat(localCopy, Backward_fan_str);
    strcat(localCopy, ",");

    strcat(localCopy, Forward_fan_str);
    strcat(localCopy, ",");

    strcat(localCopy, Left_fan_str);
    strcat(localCopy, ",");

    strcat(localCopy, Right_fan_str);
    strcat(localCopy, ",");

    strcat(localCopy, time_str);
    strcat(localCopy, ",");


    if (dataFile) {
      dataFile.println(localCopy);
      dataFile.flush();
      Serial.println(localCopy);
    }

    digitalWrite(handshake, HIGH);  // allow master
  }
}
