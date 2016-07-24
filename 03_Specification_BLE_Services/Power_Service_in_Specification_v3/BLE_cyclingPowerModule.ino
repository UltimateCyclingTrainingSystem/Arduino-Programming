void BLE_POW_updatePower(void) {

  // Power must be calculated locally, cadence remotely. Local cadence calculation only for debug purposes.
  calculateBLECadenceLocally();
  calculateBLEPowerLocally();

  // Filtering power values.
  fAverageBLEPower = filterMeasurement(fOldBLEPower, fAverageBLEPower);
  fOldBLEPower = fAverageBLEPower;

  // Prepare values to be sent by BLE
  formatBLECadence();
  formatBLEPower();

  if (bConnectedFlag && (oldBlePowerFeature != cuiBLEPowerFeatureConf)) {
    updatePowerFeatureChar();
    oldBlePowerFeature = cuiBLEPowerFeatureConf;
  }

  if (bConnectedFlag && (oldBleSensorLocation != cucBLEPowerSensorLocation)) {
    updatePowerSensorLocChar();
    oldBleSensorLocation = cucBLEPowerSensorLocation;
  }

  if (bConnectedFlag) {
    updatePowerMeasChar();
    fOldAccumulatedTorque = fAccumulatedTorque;
  }

#ifdef DEBUG_MAIN_VALUES
  Serial.print("fAverageBLETorque: ");
  Serial.print(fAverageBLETorque);
  Serial.println(" Nm");
  Serial.print("fAverageBLECadence: ");
  Serial.print(fAverageBLECadence);
  Serial.println(" rev/s");
  Serial.print("fInstCadenceRPM: ");
  Serial.print(60.0f * fAverageBLECadence);
  Serial.println(" rpm");
  Serial.print("fAverageBLEPower: ");
  Serial.print(fAverageBLEPower);
  Serial.println(" W");
  Serial.println("-------------------------------");
#endif
}

void updatePowerMeasChar(void) {
  unsigned char FLGSL = (unsigned char)(cucBLEPowerMeasurementFlags & 0x00FF);          // Measurement flags (lower byte);
  unsigned char FLGSH = (unsigned char)((cucBLEPowerMeasurementFlags >> 8) & 0x00FF);   // Measurement flags (higher byte);
  unsigned char POWL = (unsigned char)(blePowerMeasurement & 0x00FF);                   // Power measurement (lower byte);
  unsigned char POWH = (unsigned char)((blePowerMeasurement >> 8) & 0x00FF);            // Power measurement (higer byte);
  unsigned char ACCTL = 0x00;//(unsigned char)(bleAccumulatedTorque & 0x00FF);                 // Accumulated torque (lower byte);
  unsigned char ACCTH = 0x00;//(unsigned char)((bleAccumulatedTorque >> 8) & 0x00FF);          // Accumulated torque (higer byte);
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
  Serial.println("-------------------------------");
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
  Serial.println("-------------------------------");
#endif
}

void updatePowerSensorLocChar(void) {
  unsigned char SENL = (unsigned char) cucBLEPowerSensorLocation;
  unsigned char powerSensorLocCharArray[1] = { SENL };

  powerSensorLocationChar.setValue(powerSensorLocCharArray, 1);

#ifdef DEBUG_BLE_MESSAGES
  Serial.println("powerSensorLocCharArray: ");
  Serial.println(powerSensorLocCharArray[0]);
  Serial.println("-------------------------------");
#endif
}

void calculateBLECadenceLocally(void) {
  float num = float(uiCumulativeCrankRevolutions - uiOldCumulativeCrankRevolutions);
  float divi;

  // If crank event times are the same, then no more pedaling -> bike is stopped or coasting.
  if (ulCadenceLastCrankEventTime == ulOldCadenceLastCrankEventTime) {
    if (usCadenceStopCounter++ >= cusCadenceStopPeriod) {
      usCadenceStopCounter = 0;
      fAverageBLECadence = 0;
    }
  }

  // If crank event time has already wrapped around, use a modified formula.
  else if (ulOldCadenceLastCrankEventTime > ulCadenceLastCrankEventTime) {
    divi = float(64000 - ulOldCadenceLastCrankEventTime + ulCadenceLastCrankEventTime) / 1000.0f;
    fAverageBLECadence = num / divi;
    usCadenceStopCounter = 0;
  }
  // If no wrapping then proceed as usual.
  else {
    divi = float(ulCadenceLastCrankEventTime - ulOldCadenceLastCrankEventTime) / 1000.0f;
    fAverageBLECadence = num / divi;
    usCadenceStopCounter = 0;
  }

  // For the special case that the pedal stays in a position where the sensor detects.
  if (num >= 4.0f) {  // Because let's be honest, only olympians pedal that fast.
    fAverageBLECadence = 0.0f;
    uiCadenceReadWait = cuiMinRevDuration;
  }

#ifdef DEBUG_CADENCE_CALCULATIONS
  Serial.print("uiOldCumulativeCrankRevolutions: ");
  Serial.println(uiOldCumulativeCrankRevolutions);
  Serial.print("uiCumulativeCrankRevolutions: ");
  Serial.println(uiCumulativeCrankRevolutions);
  Serial.print("ulOldCadenceLastCrankEventTime: ");
  Serial.println(ulOldCadenceLastCrankEventTime);
  Serial.print("ulCadenceLastCrankEventTime: ");
  Serial.println(ulCadenceLastCrankEventTime);
  Serial.print("fAverageBLECadence: ");
  Serial.println(fAverageBLECadence);
  Serial.print("fInstCadenceRPM: ");
  Serial.println(60.0f * fAverageBLECadence);
  Serial.println("-------------------------------");
#endif

  uiOldCumulativeCrankRevolutions = uiCumulativeCrankRevolutions;
  ulOldCadenceLastCrankEventTime = ulCadenceLastCrankEventTime;
}

void calculateBLEPowerLocally(void) {
  if (usAverageRevTorqueCounter > 0) {
    fAverageBLETorque = fAverageRevTorqueSum / usAverageRevTorqueCounter;
    fAverageRevTorqueSum = 0.0f;
    usAverageRevTorqueCounter = 0;
    usPowerStopCounter = 0;
  }
  else {
    if (usPowerStopCounter++ >= cusPowerStopPeriod) {
      fAverageBLETorque = 0.0f;
      fAverageRevTorqueSum = 0.0f;
      usAverageRevTorqueCounter = 0;
      usPowerStopCounter = 0;
    }
  }

  fAverageBLEPower = 2 * cfPi * fAverageBLETorque * fAverageBLECadence;
  fAverageBLEPower = 2 * fAverageBLEPower;  // Torque is read from a single leg, multiply by two to get both legs.

#ifdef DEBUG_POWER_CALCULATIONS
  Serial.print("fAverageBLETorque: ");
  Serial.println(fAverageBLETorque);
  Serial.print("fAverageBLECadence: ");
  Serial.println(fAverageBLECadence);
  Serial.print("fAverageBLEPower: ");
  Serial.println(fAverageBLEPower);
  Serial.println("-------------------------------");
#endif
}

void formatBLECadence(void) {
  // Convert to BLE data format.
  bleCumulativeCrankRevolutions = (unsigned short) uiCumulativeCrankRevolutions; // Resolution: 1 bit = 1 revolution.
  bleLastCrankEventTime = (unsigned short)(float(ulCadenceLastCrankEventTime) * 1.024f);       // Resolution: 1 bit = 1/1024 seconds.
}

void formatBLEPower(void) {
  // Convert to BLE data format.
  blePowerMeasurement = (short) fAverageBLEPower;
  bleAccumulatedTorque = (unsigned short) (fAccumulatedTorque * 32);
}

float filterMeasurement(float oldValue, float newValue) {
  return newValue;
  //return (oldValue+newValue)/2;
}

void resetMeasurements(void) {
  fAverageBLECadence = 0.0f;
  fAverageBLETorque = 0.0f;
  fAverageRevTorqueSum = 0.0f;
  fAverageBLEPower = 0.0f;

  uiCumulativeCrankRevolutions = 0;
  uiOldCumulativeCrankRevolutions = 0;
  ulCadenceLastCrankEventTime = 0;
  ulOldCadenceLastCrankEventTime = 0;
  usCadenceStopCounter = 0;
  usAverageRevTorqueCounter = 0;
  usPowerStopCounter = 0;
}

