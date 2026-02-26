// Constants
const double R = 0.2345762060349798; // ratio between lux and rgb sensor responses
const double C_rgb = 28.803808; // Constant for normalised sensor response with distance
//const float C_lux = 19.528; // lux sensor currently not used for distance, leave here incase
const int beta = 45 * PI / 180; // lux sensor mounting angle in radians
const double denom = R * 2 * sin(beta); // here so it only need to be run once
const double n = 1.088025599603545; // fitted cos^n for rgb sensor
const int cutoff = 10; // minimum sensor value before the realtive angle is assumer >90 deg
// Cutoff value subject to further discretion

// sensor values
float VL;
float Vrgb;
float VR;
//float VL_;
float Vrgb_;
//float VR_;

// relative angular position of light source
float theta;
// relative distance between light sensors and light source. [m] <- change values of C for [cm]
float distance;

void setup() {

}

void loop() {
  // input sensor values here
  VL = 168; // left sensor (lux at beta)
  Vrgb = 547; // RGB sensor
  VR = 6; // right sensor (lux at -beta)

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
}
