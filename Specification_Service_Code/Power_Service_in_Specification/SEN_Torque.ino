void SEN_TOR_getInstantTorque(void) { 
  unsigned int uiReadInstForce;
  float fInstForce;

  // Read force signal from pin and convert analog value to a force value.
  uiReadInstForce = analogRead(cuiForcePin);
  fInstForce = float(map(uiReadInstForce, 0, 1023, 0, cuiMaxForce*100)) / 100.0f;  // Max value: 2000 N

  // Calculate the torque out of this force and the crank length.
  fInstTorque = fInstForce * cfCrankLength; // Max value aroun 200 Nm.
  fTorqueSum += fInstTorque; 
  usTorqueCounter++;

  // Save the total accumulated torque (useful for BLE). 65536 / 32 = 2048.
  fAccumulatedTorque += fInstTorque;
  if (fAccumulatedTorque >= 2048) {
    fAccumulatedTorque -= 2048;
  }

  #ifdef DEBUG_TORQUE
  Serial.print("fInstForce: ");
  Serial.println(fInstForce);
  Serial.print("fInstTorque: ");
  Serial.println(fInstTorque);
  Serial.print("fTorqueSum: ");
  Serial.println(fTorqueSum);
  Serial.print("usTorqueCounter: ");
  Serial.println(usTorqueCounter);
  Serial.print("fAccumulatedTorque: ");
  Serial.println(fAccumulatedTorque);
  #endif
}


