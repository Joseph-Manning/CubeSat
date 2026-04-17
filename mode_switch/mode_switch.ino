const int rp = 5; //receiver pin


void setup() {
  pinMode(rp, INPUT);

}

void loop() {
  int rp_val = digitalRead(rp);
  if (rp_val < 900){
  //kieran
  }
  else if (1200 < rp_val < 1400 ){
    //kieran
  }
  else if (rp_val > 1600){
    //seb -leave gyro for now
  }
  else (){
    //leave for now
  }
}
