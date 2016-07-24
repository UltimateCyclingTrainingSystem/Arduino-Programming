// Modified version of the one found in http://www.instructables.com/id/Arduino-Bike-Speedometer/?ALLSTEPS
// Thanks fto Amanda Ghassaei for the code and tutorial.

#include <CurieTimerOne.h>

// Definitions
#define SENSOR_PIN 2
//#define ADV_CALC 1 // Uncomment to get cadenceRevS averages.

// Needed variables
int sensorVal, sensorCounter;
int maxSensorCounter = 100; // Min time (in ms) of one rotation (for debouncing)
long revDuration; // Time between one full rotation (in ms)
float cadenceRevS = 0;  // Cadence in rev/s

#ifdef ADV_CALC
int revCounter = 0;
float sumCadence = 0.0f;
float avgCadence = 0.0f;
#endif

// Initialization
void setup(){
  sensorCounter = maxSensorCounter; 
  CurieTimerOne.start(1000, &cadenceRevSInt); // Interruption every 1000 us, map to desired interruption.
  pinMode(SENSOR_PIN, INPUT);
  Serial.begin(9600);
}

// Interrupt function
void cadenceRevSInt(void)
{
  // Read from sensor
  sensorVal = digitalRead(SENSOR_PIN);

  // In signal detected
  if(sensorVal) {
    // If min. time between revolutions passed -> real detection
    if (sensorCounter == 0){
      cadenceRevS = 1000/float(revDuration);
      revDuration = 0;
      sensorCounter = maxSensorCounter;

      // Calculate cadence as an average over 1 second.
      #ifdef ADV_CALC
      sumCadence += cadenceRevS;
      revCounter += 1;
      #endif
    }

    // If min. time has not passed yet -> probably caused by switch bouncing
    else {
      // Only decrease wait if wait still remaining.
      if (sensorCounter>0) {
        sensorCounter -= 1;
      }
    }
  }

  // If no signal detected
  else {
    // Only decrease wait if wait still remaining.
    if (sensorCounter>0) {
        sensorCounter -= 1;
      }
  }

  // If current revolution has lasted too long, it probably means that the bike is stopped.
  if (revDuration > 2000){
    cadenceRevS = 0;
  }

  // If bike moving, increase duration of the current revolution.
  else{
    revDuration += 1;//increment revDuration
  } 
}

void displaycadenceRevS(){
  #ifndef ADV_CALC
  Serial.println(cadenceRevS);
  #else
  avgCadence = sumCadence/float(revCounter);
  sumCadence = 0.0f;
  recCounter = 0;
  Serial.println(avgCadence);
  #endif
}

void loop(){
  //print cadence once a second.
  displaycadenceRevS();
  delay(1000);
}



