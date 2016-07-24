void SEN_CAD_getInstantCadence(void) {
  // If waiting period is over
  if (uiCadenceReadWait == 0) {
    uiCadenceReadWait = cuiMinRevDuration;

    // Increase count of events and warp around if neccesary.
    uiCumulativeCrankRevolutions++;
    if (uiCumulativeCrankRevolutions >= 65536) {
      uiCumulativeCrankRevolutions = 0;
    }

    // Get last crank event time and wrap around if neccesary at 64 seconds.
    ulCadenceLastCrankEventTime = millis() - usReferenceMillis;
    if (ulCadenceLastCrankEventTime >= 64000) {
      usReferenceMillis += 64000;
      ulCadenceLastCrankEventTime -= usReferenceMillis;
    }

    // Get average torque of the current revolution
    if(usMeasuredTorqueCounter > 0) {
      fAverageRevTorque = fMeasuredTorqueSum/usMeasuredTorqueCounter;
    }
    else {
      fAverageRevTorque = 0;
    }

    // Reset measurement sum and counter
    fMeasuredTorqueSum = 0.0f;
    usMeasuredTorqueCounter = 0;

    // Add to sum of average values
    fAverageRevTorqueSum += fAverageRevTorque;
    usAverageRevTorqueCounter++;
  }
}


