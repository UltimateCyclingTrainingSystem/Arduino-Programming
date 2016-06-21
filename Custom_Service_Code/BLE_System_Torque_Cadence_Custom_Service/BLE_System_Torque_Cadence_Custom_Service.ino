#include <CurieBLE.h>
#include <CurieTimerOne.h>

// Uncomment to use external sensor.
//#define USE_EXT_SENSOR 1

// Own sensors pins.
#define FORCE_PIN A0
#define CADENCE_PIN 2

// Bought sensor pins.
#define S_TORQUE_PIN A1
#define S_COS_PIN 4
#define S_SIN_PIN 3

// UART Communication pins.
#define TX_PIN 1
#define RX_PIN 0

// Board led (used to indicate BLE connection).
#define INT_LED_PIN 13

// Crank length
#define CRANK_LENGTH 20.0f

// Global variables.
float oldPower = 0.0f;
float oldTorque = 0.0f;
float oldCadence = 0.0f;
float power = 0.0f;
float torque = 0.0f;
float cadence = 0.0f;

int cadSensorMeas, timeWait;
int maxTimeWait = 100;
long revoDuration;

long previousMillis = 0;

// BLE Peripheral, service and caracteristic.
BLEPeripheral blePeripheral; // create peripheral instance.
BLEService powerService("19B10010-E8F2-537E-4F6C-D104768A1214"); // create service.
BLEFloatCharacteristic powerCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify); // create power characteristic and allow remote device to read and be notified.
BLEFloatCharacteristic cadenceCharacteristic("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEFloatCharacteristic torqueCharacteristic("19B10013-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);

/**
 *  Setup function, run at the beginning of execution
 */
void setup() {
  Serial.begin(9600);
  pinMode(CADENCE_PIN, INPUT);
  pinMode(INT_LED_PIN, OUTPUT);
  
  // set the local name peripheral advertises
  blePeripheral.setLocalName("PowerMeter");
  // set the UUID for the service this peripheral advertises:
  blePeripheral.setAdvertisedServiceUuid(powerService.uuid());

  // add service and characteristics
  blePeripheral.addAttribute(powerService);
  blePeripheral.addAttribute(powerCharacteristic);
  blePeripheral.addAttribute(cadenceCharacteristic);
  blePeripheral.addAttribute(torqueCharacteristic);

  // Add starting values of the characteristics
  powerCharacteristic.setValue(0.0f);
  cadenceCharacteristic.setValue(0.0f);
  torqueCharacteristic.setValue(0.0f);

  // advertise the service
  blePeripheral.begin();

  // Activate cadence interruption.
  timeWait = maxTimeWait;
  CurieTimerOne.start(1000, &readCadenceSensor);

  Serial.println("Bluetooth device active, waiting for connections...");
}

/**
 *  Main program loop.
 */
void loop() {
  // listen for BLE peripherals to connect:
  BLECentral central = blePeripheral.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());  // print the central's MAC address
    digitalWrite(INT_LED_PIN, HIGH);  // turn on the LED to indicate the connection

    // check measurements every 200ms
    // as long as the central is still connected:
    while (central.connected()) {
      long currentMillis = millis();
      // if 200ms have passed, check the heart rate measurement:
      if (currentMillis - previousMillis >= 200) {
        previousMillis = currentMillis;
        updateTorqueMeasurement();
        updateCadenceMeasurement();
        updatePowerMeasurement();
      }
    }
    // when the central disconnects, turn off the LED:
    digitalWrite(INT_LED_PIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

/**
 *  Read analog value (0-1023), map it from 0-5).
 */
void updateTorqueMeasurement(void){
  int forceMeas, forceInt;
  float forceFloat;

  forceMeas = analogRead(FORCE_PIN);
  forceInt = map(forceMeas, 0, 1023, 0, 500);
  forceFloat = float(forceInt)/10.0f;

  torque = forceFloat * CRANK_LENGTH;

  if(torque != oldTorque) {
    Serial.print("Torque is now: ");
    Serial.println(torque);
    torqueCharacteristic.setValue(torque);
    oldTorque = torque;
  }
}

/**
 * 
 */
 void updateCadenceMeasurement(void) {
  if(cadence != oldCadence) {
    Serial.print("Cadence is now: ");
    Serial.println(cadence);
    cadenceCharacteristic.setValue(cadence);
    oldCadence = cadence;
  }
 }
 
/**
 *  Calculate new power value, update old value, and notify client of a change.
 */
void updatePowerMeasurement(void) {
  power = torque*cadence;
  if(power != oldPower) {
    Serial.print("Power is now: ");
    Serial.println(power);
    powerCharacteristic.setValue(power);
    oldPower = power;
  }
}

/**
 * 
 */
void readCadenceSensor(void) {
  // Read from sensor
  cadSensorMeas = digitalRead(CADENCE_PIN);

  // In signal detected
  if(cadSensorMeas) {
    // If min. time between revolutions passed -> real detection
    if (timeWait == 0){
      cadence = 1000/float(revoDuration);
      revoDuration = 0;
      timeWait = maxTimeWait;
    }

    // If min. time has not passed yet -> probably caused by switch bouncing
    else {
      // Only decrease wait if wait still remaining.
      if (timeWait > 0) {
        timeWait -= 1;
      }
    }
  }

  // If no signal detected
  else {
    // Only decrease wait if wait still remaining.
    if (timeWait > 0) {
        timeWait -= 1;
      }
  }

  // If current revolution has lasted too long, it probably means that the bike is stopped.
  if (revoDuration > 2000){
    cadence = 0.0f;
  }

  // If bike moving, increase duration of the current revolution.
  else{
    revoDuration += 1;//increment revDuration
  } 
}

