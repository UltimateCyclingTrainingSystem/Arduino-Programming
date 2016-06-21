unsigned int readVal;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  readVal = analogRead(A5);
  
  Serial.println(readVal);
  delay(1000);
}
