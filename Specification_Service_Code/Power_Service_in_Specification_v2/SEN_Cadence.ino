void SEN_CAD_getInstantCadence(void) {
  // If waiting period is over
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
    //#ifdef DEBUG_CADENCE_MEASUREMENTS
    //    Serial.print(uiCumulativeCrankRevolutions);
    //    Serial.print(", ");
    //    Serial.println(ulCadenceLastCrankEventTime);
    //#endif
  }
  else {
    //#ifdef DEBUG_CADENCE_MEASUREMENTS
    //    Serial.println("False Positive");
    //#endif
  }
}


