//#include <Wire.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_ICM20X.h>
#include <math.h> //give shed loads of useful maths 
Adafruit_ICM20948 icm;
Adafruit_Sensor *icm_accel, *icm_gyro; 
//#define ICM_CS 10
// For software-SPI mode we need SCK/MOSI/MISO pins

#define ICM_SCK 19 //SCL is pin no. 19
//#define ICM_MISO 12
#define ICM_MOSI 0 //TBD this could transmit data to SD, Other option is to have direct connection w/ soldering.
#define ICM_SDA 18 //SDA is pin no. 18

long total_distance = 0;
long total_radians =0;
float v_i =0;
void setup() {
  Serial.begin(115200);
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:
  sensors_event_t accel; //to state the data types aquired
  sensors_event_t gyro; 
  for (int i = 0;i<100;i++){
    /* this for loop deals with distance and will break when it has travelled a certain distance*/
    int t = 25;
    icm_accel->getEvent(&accel); 
    //icm_gyro->getEvent(&gyro); //aquires normalised values of both types of data
    float a_1 = accel.acceleration.x;
    //float rs_1 = gyro.gyro.x;
    delay(t); //ensures multiplex has cycled (data will be new data)
    icm_accel->getEvent(&accel);
    //icm_gyro->getEvent(&gyro); //aquires normalised values of both types of data
    float a_2 = accel.acceleration.x;
    //float rs_2 =  gyro.gyro.x;
    v_i = (t / 1000)*(a_1+a_2);//trapezium rule. we want SI base units hence seconds. 
    //total_radians = (t / 1000)*(rs_1+rs_2)
    float x = ( (a_1 + a_2) / 2) * pow(t,2) + v_i * t ;  // this calculates the distance travelled in t accounting for current velocity
    total_distance += abs(x);
    Serial.println("/t acceleration (ms^-2)=");
    Serial.print(a_1);
    Serial.print("/t acceleration (ms^-2)=");
    Serial.print(a_2);
    Serial.print("/t instantaneuos velocity (ms^-1)");
    Serial.print(v_i);
    Serial.print("/tinstantaneous distance (m)");
    Serial.print(x);
    Serial.print("total distance so far(m)");
    Serial.print(total_distance);
    if (total_distance){}
    /*
    delay(25);
    
    icm_accel->getEvent(&accel);//aquires normalised values of both types of data
    float a_3 = accel.acceleration.x;
    float v_i_2 = 0.025*(a_2+a_3);//trapezium rule. we want SI base units hence seconds. 
    float x_i = 0.025*(v_i_1 + v_i_2);//this calculates distance crossed in t (50ms) s 
    total_distance += x_i
    if (total distance > 30){
      break
    }
    */
  }
  
}

