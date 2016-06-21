
void BLE_POW_updatePower(void) {
  // Convert to BLE data format.
  if(usTorqueCounter > 0) {
    fAverageTorque = fTorqueSum / float(usTorqueCounter);
  }
  else {
    fAverageTorque = 0.0f;
  }
  
  fTorqueSum = 0.0f;
  usTorqueCounter = 0;
  
  if(fInstCadenceRev > 0) {
    fAveragePower = fAverageTorque / fInstCadenceRev;
  }
  else {
    fAveragePower = 0.0f;
  }
  
  blePowerMeasurement = (short) fAveragePower;
  bleAccumulatedTorque = (unsigned short) (fAccumulatedTorque * 32);

  if(bConnectedFlag && (oldBlePowerFeature != cuiBLEPowerFeatureConf)) {
    updatePowerFeatureChar();
    oldBlePowerFeature = cuiBLEPowerFeatureConf;
  }
  
  if(bConnectedFlag && (oldBleSensorLocation != cucBLEPowerSensorLocation)) {
    updatePowerSensorLocChar();
    oldBleSensorLocation = cucBLEPowerSensorLocation;
  }
 
  if(bConnectedFlag && (fOldAccumulatedTorque != fAccumulatedTorque)) {
    updatePowerMeasChar(); 
    fOldAccumulatedTorque = fAccumulatedTorque;
  }

  #ifdef DEBUG_ACCUM_TORQUE
  Serial.println("fAverageTorque: ");
  Serial.println(fAverageTorque);
  Serial.println("fAveragePower: ");
  Serial.println(fAveragePower);
  Serial.println("blePowerMeasurement: ");
  Serial.println(blePowerMeasurement);
  Serial.println("bleAccumulatedTorque: ");
  Serial.println(bleAccumulatedTorque);
  #endif
}

void updatePowerMeasChar(void) {
  unsigned char FLGSL = (unsigned char)(cucBLEPowerMeasurementFlags & 0x00FF);          // Measurement flags (lower byte);
  unsigned char FLGSH = (unsigned char)((cucBLEPowerMeasurementFlags >> 8) & 0x00FF);   // Measurement flags (higher byte);
  unsigned char POWL = (unsigned char)(blePowerMeasurement & 0x00FF);                   // Power measurement (lower byte);
  unsigned char POWH = (unsigned char)((blePowerMeasurement >> 8) & 0x00FF);            // Power measurement (higer byte);
  unsigned char ACCTL = (unsigned char)(bleAccumulatedTorque & 0x00FF);                 // Accumulated torque (lower byte);
  unsigned char ACCTH = (unsigned char)((bleAccumulatedTorque >> 8) & 0x00FF);          // Accumulated torque (higer byte);
  unsigned char CCRL = (unsigned char)((bleCumulativeCrankRevolutions) & 0x00FF);       // Cumulative crank revolutions (lower byte);
  unsigned char CCRH = (unsigned char)((bleCumulativeCrankRevolutions >> 8) & 0x00FF);  // Cumulative crank revolutions (higher byte);
  unsigned char LCETL = (unsigned char)((bleLastCrankEventTime) & 0x00FF);              // Last event time (lower byte);
  unsigned char LCETH = (unsigned char)((bleLastCrankEventTime >> 8) & 0x00FF);         // Last event time (higher byte);
  unsigned char powerMeasurementCharArray[10] = { FLGSL, FLGSH, POWL, POWH, ACCTL, ACCTH, CCRL, CCRH, LCETL, LCETH };

   powerMeasurementChar.setValue(powerMeasurementCharArray, 10);

  #ifdef DEBUG_BLE_MESSAGES
  Serial.println("powerMeasurementCharArray: ");
  Serial.println(powerMeasurementCharArray[0]);
  Serial.println(powerMeasurementCharArray[1]);
  Serial.println(powerMeasurementCharArray[2]);
  Serial.println(powerMeasurementCharArray[3]);
  Serial.println(powerMeasurementCharArray[4]);
  Serial.println(powerMeasurementCharArray[5]);
  Serial.println(powerMeasurementCharArray[6]);
  Serial.println(powerMeasurementCharArray[7]);
  Serial.println(powerMeasurementCharArray[8]);
  Serial.println(powerMeasurementCharArray[9]);
  #endif
}

void updatePowerFeatureChar(void) {
  unsigned char PWFLL = (unsigned char)(cuiBLEPowerFeatureConf & 0x000000FF);
  unsigned char PWFLH = (unsigned char)((cuiBLEPowerFeatureConf >> 8) & 0x000000FF);
  unsigned char PWFHL = (unsigned char)((cuiBLEPowerFeatureConf >> 16) & 0x000000FF);
  unsigned char PWFHH = (unsigned char)((cuiBLEPowerFeatureConf >> 24) & 0x000000FF);
  unsigned char powerFeatureCharArray[4] = { PWFLL, PWFLH, PWFHL, PWFHH };
  
  powerFeatureChar.setValue(powerFeatureCharArray, 4);

  #ifdef DEBUG_BLE_MESSAGES
  Serial.println("powerFeatureCharArray: ");
  Serial.println(powerFeatureCharArray[0]);
  Serial.println(powerFeatureCharArray[1]);
  Serial.println(powerFeatureCharArray[2]);
  Serial.println(powerFeatureCharArray[3]);
  #endif
}

void updatePowerSensorLocChar(void) {
  unsigned char SENL = (unsigned char) cucBLEPowerSensorLocation;
  unsigned char powerSensorLocCharArray[1] = { SENL };
  
  powerSensorLocationChar.setValue(powerSensorLocCharArray, 1);

  
  #ifdef DEBUG_BLE_MESSAGES
  Serial.println("powerSensorLocCharArray: ");
  Serial.println(powerSensorLocCharArray[0]);
  #endif
}

