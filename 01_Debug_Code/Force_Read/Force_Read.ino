unsigned int valRead;
unsigned int voltage;
float voltageFloat = 0;
float estForce = 0;

const float slope = 261.7427;
const float offset = -181.948;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
  valRead = analogRead(A5);
  voltage = map(valRead, 0, 1023, 0, 500);
  voltageFloat = float(voltage)/100.0f;
  estForce = slope*voltageFloat+offset;
  if (estForce < 0.0f) {
    estForce = 0.0f;
  }
  Serial.println(valRead);
  Serial.println(voltageFloat);
  Serial.println(estForce);
  Serial.println("Weight: ");
  Serial.println(2*estForce/9.81);
  Serial.println("-----------------");
  delay(1000);
}
