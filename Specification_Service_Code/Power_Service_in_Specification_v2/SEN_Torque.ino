void SEN_TOR_getInstantTorque(void) {
  unsigned int uiReadInstVoltage;
  float fInstVoltage, fInstForce;

  // Read force signal from pin and convert analog value to a force value.
  uiReadInstVoltage = analogRead(cuiForcePin);
  fInstVoltage = float(map(uiReadInstVoltage, 0, 1023, 0, 500)) / 100.0f;
  fInstForce = fForceSlope*fInstVoltage+fForceOffset;

  // Calculate the torque out of this force and the crank length.
  fInstTorque = fInstForce * cfCrankLength; // Max value around 200 Nm.
  fTorqueSum += fInstTorque;
  usTorqueCounter++;

  // Save the total accumulated torque (useful for BLE). 65536 / 32 = 2048.
  fAccumulatedTorque += fInstTorque;
  if (fAccumulatedTorque >= 2048) {
    fAccumulatedTorque -= 2048;
  }

  // Debug force measurement.
#ifdef DEBUG_TORQUE_MEASUREMENTS
  Serial.print("uiReadInstVoltage: ");
  Serial.println(uiReadInstVoltage);
  Serial.print("fInstForce: ");
  Serial.println(fInstForce);
  Serial.print("fInstTorque: ");
  Serial.println(fInstTorque);
  Serial.println("-------------------------------");
#endif
}


