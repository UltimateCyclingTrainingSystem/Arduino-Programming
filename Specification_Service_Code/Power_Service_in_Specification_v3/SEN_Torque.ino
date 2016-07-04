void SEN_TOR_getInstantTorque(void) {
  unsigned int uiMeasuredVoltage;
  float fMeasuredVoltage, fMeasuredForce;

  // Read force signal from pin and convert analog value to a force value.
  uiMeasuredVoltage = analogRead(cuiForcePin);
  fMeasuredVoltage = float(map(uiMeasuredVoltage, 0, 1023, 0, 500)) / 100.0f;
  fMeasuredForce = fForceSlope*fMeasuredVoltage+fForceOffset;

  // Ignore negative forces
  if(fMeasuredForce < 0.0f) {
    fMeasuredForce = 0.0f;
  }

  // Calculate the torque out of this force and the crank length.
  fMeasuredTorque = fMeasuredForce * cfCrankLength; // Max value around 200 Nm.
  fMeasuredTorqueSum += fMeasuredTorque;
  usMeasuredTorqueCounter++;

  // Debug force measurement.
#ifdef DEBUG_TORQUE_MEASUREMENTS
  Serial.print("uiMeasuredVoltage: ");
  Serial.println(uiMeasuredVoltage);
  Serial.print("fMeasuredForce: ");
  Serial.println(fMeasuredForce);
  Serial.print("fMeasuredTorque: ");
  Serial.println(fMeasuredTorque);
  Serial.println("-------------------------------");
#endif
}

