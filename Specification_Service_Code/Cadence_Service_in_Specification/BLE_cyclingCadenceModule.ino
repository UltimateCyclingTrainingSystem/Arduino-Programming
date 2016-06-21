void BLE_CAD_updateCadence(void) {
  // Convert to BLE data format.
  bleCumulativeCrankRevolutions = (unsigned short) uiCumulativeCrankRevolutions; // Resolution: 1 bit = 1 revolution.
  bleLastCrankEventTime = (unsigned short)(float(ulCadenceLastCrankEventTime)*1.024f);         // Resolution: 1 bit = 1/1024 seconds.

  // Update measurements characteristic.
  if(bConnectedFlag && (uiCumulativeCrankRevolutions != uiOldCumulativeCrankRevolutions && ulCadenceLastCrankEventTime != ulOldCadenceLastCrankEventTime)) {
    updateCadenceMeasChar();
  }

  // Update feature characteristic.
  if(bConnectedFlag && (usOldCadenceFeature != cusBLECadenceFeatureConf)) {
    updateCadenceFeatureChar();
  }

  // Calculate cadence in the way BLE intended.
  calculateCadence();

  // Update old values.
  usOldCadenceFeature =  cusBLECadenceFeatureConf;
  uiOldCumulativeCrankRevolutions = uiCumulativeCrankRevolutions;
  ulOldCadenceLastCrankEventTime = ulCadenceLastCrankEventTime;
}

void updateCadenceMeasChar(void) {
  unsigned char FLGS = (unsigned char) cucBLECadenceMeasurementFlags;
  unsigned char CCRL = (unsigned char)((bleCumulativeCrankRevolutions) & 0x00FF);
  unsigned char CCRH = (unsigned char)((bleCumulativeCrankRevolutions >> 8) & 0x00FF);
  unsigned char LCETL = (unsigned char)((bleLastCrankEventTime) & 0x00FF);
  unsigned char LCETH = (unsigned char)((bleLastCrankEventTime >> 8) & 0x00FF);

  unsigned char cadenceMeasurementCharArray[5] = { FLGS, CCRL, CCRH, LCETL, LCETH };

  #ifdef DEBUG_BLE_MESSAGES
  Serial.println("cadenceMeasurementCharArray: ");
  Serial.println(cadenceMeasurementCharArray[0]);
  Serial.println(cadenceMeasurementCharArray[1]);
  Serial.println(cadenceMeasurementCharArray[2]);
  Serial.println(cadenceMeasurementCharArray[3]);
  Serial.println(cadenceMeasurementCharArray[4]);
  #endif
  
  cadenceMeasurementChar.setValue(cadenceMeasurementCharArray, 5);
}


void updateCadenceFeatureChar(void) {
  unsigned char FCL = (unsigned char)(cusBLECadenceFeatureConf & 0x00FF);
  unsigned char FCH = (unsigned char)((cusBLECadenceFeatureConf >> 8) & 0x00FF);

  unsigned char cadenceFeatureCharArray[2] = { FCL, FCH };

  #ifdef DEBUG_BLE_MESSAGES
  Serial.println("cadenceFeatureCharArray: ");
  Serial.println(cadenceFeatureCharArray[0]);
  Serial.println(cadenceFeatureCharArray[1]);
  #endif
  
  cadenceFeatureChar.setValue(cadenceFeatureCharArray, 2);
}

void calculateCadence(void) {
  float num = float(uiCumulativeCrankRevolutions - uiOldCumulativeCrankRevolutions);
  float divi;

  // If crank event time has already wrapped around, use a modified formula.
  if (ulOldCadenceLastCrankEventTime > ulCadenceLastCrankEventTime) {
    divi = float(64000 - ulOldCadenceLastCrankEventTime + ulCadenceLastCrankEventTime) / 1000.0f;
  }
  // If no wrapping then proceed as usual.
  else {
    divi = float(ulCadenceLastCrankEventTime - ulOldCadenceLastCrankEventTime) / 1000.0f;
  }
  
  // If crank event times are the same, then no more pedaling -> bike is stopped or coasting.
  if (ulCadenceLastCrankEventTime == ulOldCadenceLastCrankEventTime) {
    fInstCadenceRev = 0;
  }
  // If different times -> bike moving.
  else {
    fInstCadenceRev = num / divi;
  }

  // For the special case that the pedal stays in a position where the sensor detects.
  if (fInstCadenceRev >= 4.0f) {  // Because let's be honest, only olympians pedal that fast.
    fInstCadenceRev = 0.0f;
    uiCadenceReadWait = cuiMinRevDuration;
  }

  #ifdef DEBUG_CADENCE_VALUES
  Serial.print("uiCumulativeCrankRevolutions: ");
  Serial.println(uiCumulativeCrankRevolutions);
  Serial.print("uiOldCumulativeCrankRevolutions: ");
  Serial.println(uiOldCumulativeCrankRevolutions);
  Serial.print("ulCadenceLastCrankEventTime: ");
  Serial.println(ulCadenceLastCrankEventTime);
  Serial.print("ulOldCadenceLastCrankEventTime: ");
  Serial.println(ulOldCadenceLastCrankEventTime);
  Serial.print("Cadence: ");
  Serial.println(fInstCadenceRev);
  #endif
}

