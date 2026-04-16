#include <MAVLink.h>
#include <SoftwareSerial.h>
//3.3 and 5 V pins
const int stepd = 2;
const int stepu = 3;
SoftwareSerial pixSerial(10,11); //RX,TX

void setup() {
  Serial.begin(57600);
  pixSerial.begin(57600); //if this is unstable go to 19200 range
  pinMode(stepu, OUTPUT);
  pinMode(stepd, OUTPUT);
  digitalWrite(stepd, LOW);
  digitalWrite(stepu, LOW);
  //const voltage pins
  digitalWrite(stepd, HIGH);
  digitalWrite(stepu, HIGH);
}

void loop() {
  mavlink_message_t msg;
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];

  mavlink_msg_heartbeat_pack(
      1,
      MAV_COMP_ID_AUTOPILOT1,
      &msg,
      MAV_TYPE_QUADROTOR,
      MAV_AUTOPILOT_GENERIC,
      MAV_MODE_FLAG_MANUAL_INPUT_ENABLED,
      0,
      MAV_STATE_STANDBY
  );

  uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);

  // Send to Pixhawk
  pixSerial.write(buf, len);

  // Debug print in HEX so it's readable
  for (int i = 0; i < len; i++) {
      Serial.print(buf[i], HEX);
      Serial.print(" ");
  }
  Serial.println();

  delay(1000);
}