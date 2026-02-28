//Libs and their pronouns
#include <Wire.h>
#include "Adafruit_VEML7700.h"
#include "Adafruit_TCS34725.h"

// Constants
//const double R = 0.2345762060349798; // ratio between lux and rgb sensor responses
const float R = 0.5; // value depends on gain
const double C_rgb = 28.803808; // Constant for normalised sensor response with distance
//const float C_lux = 19.528; // lux sensor currently not used for distance, leave here incase
const float beta = 45 * PI / 180; // lux sensor mounting angle in radians
const double denom = R * 2 * sin(beta); // here so it only need to be run once
//const double denom = 3; // here so it only need to be run once
const double n = 1.088025599603545; // fitted cos^n for rgb sensor
const int cutoff = 10; // minimum sensor value before the realtive angle is assumer >90 deg
// Cutoff value subject to further discretion

#define MUX_ADDR 0x70
// Sensor objects
Adafruit_VEML7700 veml = Adafruit_VEML7700(); // lux
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X); // RGB
// RGB very sensitive to interger overflow / saturation -> these values work with my phone flashlight
// the test light may be brighter
// Datasheet recommends dark plastic filter over sensor

// sensor values
float VL;
double Vrgb;
float VR;
//float VL_;
float Vrgb_;
//float VR_;

// relative angular position of light source
double theta;
// relative distance between light sensors and light source. [m] <- change values of C for [cm]
float distance;

// --- Subroutines ---
void selectChannel(uint8_t i) {
  // Select Multiplex channel to connect to 
  if (i > 3) return;
  Wire.beginTransmission(MUX_ADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

void setRGB_LED(bool on) {
  // Toggle RGB sensor LED
  selectChannel(2);
  // Passing false turns LED OFF, true turns LED ON
  tcs.setInterrupt(on);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // --- Initialize Lux Ports ---
  selectChannel(1);
  if (!veml.begin()) {Serial.println("VEML7700 not found on Port 1");}
  else {Serial.println("Lux on port 1 found");}

  selectChannel(3);
  if (!veml.begin()) {Serial.println("VEML7700 not found on Port 3");}
  else {Serial.println("Lux on port 3 found");}

  // --- Initialize RGB Port ---
  selectChannel(2);
  if (!tcs.begin()) {
    Serial.println("RGB sensor not found on Port 2");
  } else {
    Serial.println("RGB sensor found on Port 2");
  }
  // Switch LED off (default is on)
  setRGB_LED(false);
}

void loop() {
  // input sensor values here
  selectChannel(1);
  VL = veml.readLux(); // left sensor (lux at beta)
  Serial.print("P1 Lux: "); Serial.println(VL);
  
  selectChannel(2);
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  Serial.println(r);
  Serial.println(g);
  Serial.println(b);
  Serial.println(c);
  // for the calculate lux function, if one of the values is zero (typically green)
  // the normalisation does a div 0 and Vrgb goes to its maximum.
  // Check if g is zero and add 1
  if (g == 0) {g=1;}
  Vrgb = tcs.calculateLux(r, g, b); // RGB sensor
  Serial.print("P2 RGB: "); Serial.println(Vrgb);
  //setRGB_LED(false);
  tcs.setInterrupt(true);
  if (c >= 65535 || r >= 65535 || g >= 65535 || b >= 65535) {
    Serial.println("Sensor Saturated! Reduce Gain or Integration Time.");
  } 
  
  
  selectChannel(3);
  VR = veml.readLux(); // right sensor (lux at -beta)
  Serial.print("P3 Lux: "); Serial.println(VR);

  // Evalute sensor values to determine if light is within FOV (+-90 deg)
  // By considering VL as the first condition, the craft will always turn left if both L and R
  // sensors are in the dark -> may be best to consider sensor history and turn towards
  // the sensor that went dark last
  if (VL < cutoff) {
    // Turn left, dont calc theta or distance
    theta = 0;
    distance = 0;
  } else if (VR < cutoff) {
    // Turn right
    theta = 0;
    distance = 0;
  } else {
    Serial.println("Light source in view");
    // in view of L and R -> in view of rgb
    // calc theta and distance
    theta = atan((VL - VR) / (denom * Vrgb)); // value for theta in radians
    // Find normalised sensor values for distance calc
    // If cos = 0 then div 0
    // cos 0 -> light out of FOV, this code block shouldnt run
    // Only considering distance from rgb sensor for now,
    // until lux sensor accuracy can be improved
    //VL_ = VL / cos(theta + beta);
    Vrgb_ = Vrgb / pow(cos(theta), n); // power is computational intensive, but huge boost to accuracy
    //VR_ = VR / cos(theta - beta);

    distance = C_rgb / pow(Vrgb_, 0.5);
  }
  Serial.print("Theta: "); Serial.println(theta * 180 / PI);
  Serial.println("====================");
  delay(1000); // delay for readability
}
