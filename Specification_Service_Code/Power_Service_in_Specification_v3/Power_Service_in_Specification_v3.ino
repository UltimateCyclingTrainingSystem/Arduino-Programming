#include <CurieBLE.h>
#include <CurieTimerOne.h>

//IMPORTANT - Battery operation//
// Comment to debug with USB (different calibration values).
#define BATTERY_OP  1

// Uncomment for debugging.
//#define DEBUG_CADENCE_MEASUREMENTS 1
//#define DEBUG_CADENCE_CALCULATIONS 1
//#define DEBUG_TORQUE_MEASUREMENTS 1
//#define DEBUG_POWER_CALCULATIONS 1
//#define DEBUG_BLE_PERIPHERAL 1
//#define DEBUG_BLE_MESSAGES 1
//#define DEBUG_MAIN_VALUES 1

/// START DEFINITIONS ///
// Own sensors pins.
const unsigned int cuiForcePin = 5;
const unsigned int cuiCadencePin = 2;

// Board led (used to indicate BLE connection).
const unsigned int cuiInternalLedPin = 13;

// Calculation constants.
const float cfCrankLength = 0.17f;   // in m.
const float cfPi = 3.1416f;
#ifdef BATTERY_OP   // Battery Calibration
const float fForceSlope = 261.7427f;
const float fForceOffset = -181.948f;
#else // USB Calibration
const float fForceSlope = 290.10f;
const float fForceOffset = -187.26f;
#endif

// Timer constants
const unsigned long culTimerInterruptPeriod = 1000; // in us.
const unsigned long culTimerTorqueReadPeriod = 1;  // in ms.
const unsigned long culBLEUpdatePeriod = 1000;      // in ms.
const unsigned int cuiMinRevDuration = 250;         // in ms.
const unsigned short cusCadenceStopPeriod = 3;      // in s.
const unsigned short cusPowerStopPeriod = 3;      // in s.

// BLE Configuration values
const unsigned char cucBLEPowerSensorLocation = 0x06;     // Right Crank.
const unsigned char cucBLECadenceMeasurementFlags = 0x02; // Crank revolution data present.
const unsigned short cusBLECadenceFeatureConf = 0x0002;   // Crank revolution data present.
const unsigned short cucBLEPowerMeasurementFlags = 0x002C; // Acumulated Torque, and crank revolutions present.
const unsigned int cuiBLEPowerFeatureConf = 0x0010000A;    //  Acumulated Torque, and crank revolutions present, force based measurements,

// Global variables
// Connection state flag.
bool bConnectedFlag = 0;

// Timer counters.
unsigned long ulTorqueMillisecondCounter = 0;
volatile unsigned int uiCadenceReadWait = cuiMinRevDuration;

// Values updates every measurement period.
float fMeasuredTorque = 0.0f;
float fMeasuredTorqueSum = 0.0f;
unsigned short usMeasuredTorqueCounter = 0;

// Values updated every revolution (inside an interruption, so we use volatile).
volatile unsigned long usReferenceMillis;
volatile unsigned long ulCadenceLastCrankEventTime = 0;
volatile unsigned int uiCumulativeCrankRevolutions = 0;
volatile float fAverageRevTorque = 0.0f;
volatile float fAverageRevTorqueSum = 0.0f;
volatile unsigned short usAverageRevTorqueCounter = 0;

// Values updated every BLE period.
float fAverageBLECadence = 0.0f;
float fAverageBLEPower = 0.0f;
float fAverageBLETorque = 0.0f;
float fOldBLEPower = 0.0f;
unsigned int uiOldCumulativeCrankRevolutions = 0;
unsigned long ulOldCadenceLastCrankEventTime = 0;
unsigned short usCadenceStopCounter = 0;
unsigned short usPowerStopCounter = 0;


// BLE Formatted global variables
unsigned short bleCumulativeCrankRevolutions = 0; // Resolution: 1 bit = 1 revolution.
unsigned short bleLastCrankEventTime = 0;         // Resolution: 1 bit = 1/1024 seconds.
short blePowerMeasurement = 0;                    // Resolution: 1 bit = 1 Watt.
unsigned short bleAccumulatedTorque = 0;          // Resolution: 1 bit = 1/32 Nm.

// Variables used for updating BLE values only if some change is detected.
float fAccumulatedTorque = 0;
float fOldAccumulatedTorque = 0;
unsigned int oldBlePowerFeature = 0;
unsigned char oldBleSensorLocation = 0;

// BLE Peripheral, service and caracteristic.
BLEPeripheral blePeripheral; // create peripheral instance.

// BLE Cycing power initialization.
BLEService powerService("1818");
BLECharacteristic powerMeasurementChar("2A63", BLENotify, 10);
BLECharacteristic powerFeatureChar("2A65", BLERead, 4);
BLECharacteristic powerSensorLocationChar("2A5D", BLERead, 1);

/// END DEFINITIONS ///

/**
    Setup function, run at the beginning of execution
*/
void setup() {
  // Start serial debugging.
  Serial.begin(250000);

  // Initialize digital pin behaviour.
  pinMode(cuiCadencePin, INPUT);
  pinMode(cuiInternalLedPin, OUTPUT);
  //analogReference(DEFAULT);

  // Initialize BLE device, services and characteristics.
  initBleDevice();

  // Initialize cadence interruptions
  attachInterrupt(cuiCadencePin, SEN_CAD_getInstantCadence, RISING);

  // Initialize timer interruptions.
  CurieTimerOne.start(culTimerInterruptPeriod, &Int_1ms);

  // Advertise the service
  blePeripheral.begin();
  Serial.println("Bluetooth device active, waiting for connections...");
}

/**
    Main program loop.
*/
void loop() {
  // listen for BLE peripherals to connect:
  BLECentral central = blePeripheral.central();
  Serial.println("Waiting for client");

#ifdef DEBUG_BLE_PERIPHERAL
  Serial.print("BLEPeripheral: ");
  Serial.println(blePeripheral.connected());
  Serial.print("BLECentral: ");
  Serial.println(central.connected());
#endif

  delay(1000);

  // if a central is connected to peripheral:
  if (central) {
    // print the central's MAC address
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    // Indicate succesfull connection with a flag and with the on board led.
    bConnectedFlag = 1;
    resetMeasurements();
    //digitalWrite(cuiInternalLedPin, HIGH);  // turn on the LED to indicate the connection

    // Loop here while central is still connected
    while (central.connected()) {

      // Update power and cadence characteristics every second.
      Serial.println("Running");
      BLE_POW_updatePower();

#ifdef DEBUG_BLE_PERIPHERAL
      Serial.print("BLEPeripheral: ");
      Serial.println(blePeripheral.connected());
      Serial.print("BLECentral: ");
      Serial.println(central.connected());
#endif
      delay(culBLEUpdatePeriod);
    }

    // when the central disconnects, turn off the LED and reset the flag.
    bConnectedFlag = 0;
    resetMeasurements();
    //digitalWrite(cuiInternalLedPin, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    central.disconnect();

    Serial.println("Bluetooth device active, waiting for connections...");
  }
}

void Int_1ms(void) {
  if (bConnectedFlag) {
    if (uiCadenceReadWait > 0) {
      uiCadenceReadWait--;
    }
    ulTorqueMillisecondCounter++;
    if (ulTorqueMillisecondCounter >= culTimerTorqueReadPeriod) {
      ulTorqueMillisecondCounter = 0;
      SEN_TOR_getInstantTorque();
    }
  }
}

void initBleDevice(void) {
  // Initialize BLE device, services and characteristics.
  blePeripheral.setLocalName("UCTS_BLE");
  blePeripheral.setAdvertisedServiceUuid(powerService.uuid());

  // Initialize power service.
  blePeripheral.addAttribute(powerService);
  blePeripheral.addAttribute(powerMeasurementChar);
  blePeripheral.addAttribute(powerFeatureChar);
  blePeripheral.addAttribute(powerSensorLocationChar);

  // Initialize timer interruptions.
  CurieTimerOne.start(culTimerInterruptPeriod, &Int_1ms);

  // Advertise the service
  blePeripheral.begin();
  Serial.println("Bluetooth device active, waiting for connections...");
  delay(1000);
}


