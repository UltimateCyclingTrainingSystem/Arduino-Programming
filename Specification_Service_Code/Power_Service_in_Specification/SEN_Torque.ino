void SEN_TOR_getInstantTorque(void) {
  unsigned int uiReadInstForce;
  float fInstForce;

  // Read force signal from pin and convert analog value to a force value.
  uiReadInstForce = analogRead(cuiForcePin);
  fInstForce = float(map(uiReadInstForce, 0, 1023, 0, cuiMaxForce * 100)) / 100.0f; // Max value: 2000 N

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
  Serial.print("uiReadInstForce: ");
  Serial.println(uiReadInstForce);
  Serial.print("fInstForce: ");
  Serial.println(fInstForce);
  Serial.print("fInstTorque: ");
  Serial.println(fInstTorque);
  Serial.println("-------------------------------");
#endif
}


