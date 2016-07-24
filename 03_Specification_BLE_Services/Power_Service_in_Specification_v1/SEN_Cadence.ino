void SEN_CAD_getInstantCadence(void) {
  bool bReadCadence = 0;

  bReadCadence = digitalRead(cuiCadencePin); // Read from sensor

  // Debug cadence measurements.
#ifdef DEBUG_CADENCE_MEASUREMENTS
  Serial.print("bReadCadence: ");
  Serial.println(bReadCadence);
#endif

  if (bReadCadence) { // In signal detected
    // If waiting time is over, it means that this is a real measurement.
    if (uiCadenceReadWait == 0) {
      uiCadenceReadWait = cuiMinRevDuration;
      // Increase count of events and warp around if neccesary.
      uiCumulativeCrankRevolutions++;
      if (uiCumulativeCrankRevolutions >= 65536) {
        uiCumulativeCrankRevolutions = 0;
      }

      // Get last crank event time and wrap around if neccesary.
      ulCadenceLastCrankEventTime = millis() - usReferenceMillis;
      if (ulCadenceLastCrankEventTime >= 64000) {
        usReferenceMillis += 64000;
        ulCadenceLastCrankEventTime -= usReferenceMillis;
      }
#ifdef DEBUG_CADENCE_MEASUREMENTS
      Serial.print(uiCumulativeCrankRevolutions);
      Serial.print(", ");
      Serial.println(ulCadenceLastCrankEventTime);
#endif
    }

    // if waiting time is not over, then it was a false positive (bounce) read.
    else {
      // Reduce the waiting time if nothing is detected.
      if (uiCadenceReadWait > culTimerCadenceReadPeriod) {
        uiCadenceReadWait -= culTimerCadenceReadPeriod;
      }
      else if (uiCadenceReadWait > 0) {
        uiCadenceReadWait = 0;
      }
#ifdef DEBUG_CADENCE_MEASUREMENTS
      Serial.println("False positive");
#endif
    }
#ifdef DEBUG_CADENCE_MEASUREMENTS
    Serial.println("-------------------------------");
#endif
  }

  else { // If no signal detected
    // Reduce the waiting time if nothing is detected.
    if (uiCadenceReadWait > culTimerCadenceReadPeriod) {
      uiCadenceReadWait -= culTimerCadenceReadPeriod;
    }
    else if (uiCadenceReadWait > 0) {
      uiCadenceReadWait = 0;
    }
  }
}


