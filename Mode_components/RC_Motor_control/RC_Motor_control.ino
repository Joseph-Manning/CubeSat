//pin set up
//motor control
const int IN1 = 6;  //only need to be H/L //IN1
const int IN2 = 7;  //only need to be H/L //IN2
const int ENA = 5;
const int logic = 4;
//receive pin
const int rp = 3;  //receiver pin
const int RCRW = 9;

//variables
//reaction wheel control
int RW_IN;
int RW_PWM;
//mode detection
int rp_val;

void setup() {
  Serial.begin(9600);
  pinMode(rp, INPUT);
  pinMode(RCRW, INPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(logic, OUTPUT);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
  digitalWrite(logic, 1);
}

void loop() {
  rp_val = pulseIn(rp, 1, 30000);
  if (rp_val < 900) {
    //kieran
  } else if (rp_val > 1200 && rp_val < 1400) {
    //kieran
  } else if (rp_val > 1600) {
    RW_IN = pulseIn(RCRW, 1, 30000);
    if (RW_IN < 1452) {
      RW_IN = constrain(RW_IN, 982, 1452);
      digitalWrite(IN1, 0);
      digitalWrite(IN2, 1);
      int signal_in = 1452 - RW_IN;  // this wraps correctly
      RW_PWM = signal_in * 0.54;
      round(RW_PWM);
    } else if (RW_IN > 1532) {
      RW_IN = constrain(RW_IN, 1532, 2002);
      digitalWrite(IN1, 1);
      digitalWrite(IN2, 0);
      int signal_in = RW_IN - 1532;
      RW_PWM = signal_in * 0.54;
      round(RW_PWM);
    } else {
      digitalWrite(IN1, 0);
      digitalWrite(IN2, 0);
      RW_PWM = 0;
    }
    RW_PWM = constrain(RW_PWM, 100, 255);
    analogWrite(ENA, RW_PWM);
    Serial.println(RW_IN);
    Serial.println(RW_PWM);
    //982 1480, 1500 2002


  } else {
    //leave for now
  }
}
