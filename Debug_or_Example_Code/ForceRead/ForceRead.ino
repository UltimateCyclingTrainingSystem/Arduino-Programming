unsigned int valRead;
unsigned int voltage;
float voltageFloat = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  valRead = analogRead(A5);
  voltage = map(valRead, 0, 1023, 0, 500);
  voltageFloat = float(voltage)/100.0f;
  Serial.println(voltage);
  Serial.println(voltageFloat);
  delay(1000);
}
