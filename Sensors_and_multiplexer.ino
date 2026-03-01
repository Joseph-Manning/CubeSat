#include <Wire.h>
#include <Adafruit_VEML7700.h>
#include <Adafruit_TCA9548A.h>
#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>

Adafruit_TCS34725 tcs = Adafruit_TCS34725( //RGB sensor
TCS34725_INTEGRATIONTIME_100MS,      //Sets the time spent recording on RGB sensor
TCS34725_GAIN_1X                    //Sets the gain of the RGB sensor
);

Adafruit_TCA9548A mux = Adafruit_TCA9548A();  //Multiplexer

Adafruit_ICM20948 icm; // 9 DOF sensor

Adafruit_VEML7700 veml1 = Adafruit_VEML7700(); //Lux sensor 2
Adafruit_VEML7700 veml2 = Adafruit_VEML7700(); //Lux sensor 1

void setup() {
Serial.begin(115200);
Wire.begin();

if (!mux.begin()) {
Serial.println("Multiplexer not found");
 while (1);
}
// Checking for lux 1-----------------------------
mux.select(0); //Cycles through multiplexer channels
if (!veml1.begin()) { //Starts the sensor
Serial.println("Sensor lux 1 not found");
while (1);
}
Serial.println("Sensor lux 1 found");
//Sets the gain and iteration time of lux 1
veml1.setGain(VEML7700_GAIN_1);
veml1.setIntegrationTime(VEML7700_IT_100MS);

// Checking for lux 2-----------------------------
mux.select(1);
if (!veml2.begin()) {
Serial.println("Sensor lux 2 not found");
while (1);
}
Serial.println("Sensor lux 2 found");
//Sets the gain and integration time of lux 2
veml2.setGain(VEML7700_GAIN_1);
veml2.setIntegrationTime(VEML7700_IT_100MS);

//Checking if RGB sensor is found------------------
mux.select(2);
if (!tcs.begin()) {
Serial.println("Sensor RGB not found");
while (1);
}
Serial.println("Sensor RGB found");

//Checking if 9 DOF sensor found--------------------
mux.select(3);
if (!icm.begin_I2C()) {
Serial.println("9 DOF sensor not found");
while (1);
}
Serial.println("9 DOF sensor found");
icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G); //Sets max acceleration rate measureable
icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS); // Sets max rotation rate measureable

}

void loop() {
float lux1, lux2, // Setting variables
sensors_event_t accel, gyro //Creating data storage lists

mux.select(0); //Getting the value from lux 1
lux1 = readLux();
Serial.print("Lux 1: "); //Returning the values from lux 1 and 2
Serial.print(lux1);

mux.select(1); //Getting the value from lux 2
lux2 = readLux();
Serial.print("   Lux 2: ");
Serial.println(lux2);

mux.select(3);
icm.getEvent(&accel, &gyro, &temp, &mag);
// Accelerometer readings--------------------
Serial.print("Accel X: ");
Serial.print(accel.acceleration.x);
Serial.print("  Y: ");
Serial.print(accel.acceleration.y);
Serial.print("  Z: ");
Serial.println(accel.acceleration.z);
// Gyroscope readings------------------------
/* Serial.print("Gyro X: ");
Serial.print(gyro.gyro.x);
Serial.print("  Y: ");
Serial.print(gyro.gyro.y); */
Serial.print("  Z: ");
Serial.println(gyro.gyro.z);

delay(100); //The delay between loops
}
