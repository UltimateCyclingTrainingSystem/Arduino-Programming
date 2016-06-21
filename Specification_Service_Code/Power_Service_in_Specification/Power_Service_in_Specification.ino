#include <CurieBLE.h>
#include <CurieTimerOne.h>

// Uncomment for debugging.
//#define DEBUG_CADENCE_MEASUREMENTS 1
//#define DEBUG_CADENCE_VALUES 1
//#define DEBUG_TORQUE 1
//#define DEBUG_BLE_PERIPHERAL 1
//#define DEBUG_BLE_MESSAGES 1
//#define DEBUG_ACCUM_TORQUE 1

/// START DEFINITIONS ///
// Own sensors pins.
const unsigned int cuiForcePin = 5;
const unsigned int cuiCadencePin = 2;

// Board led (used to indicate BLE connection).
const unsigned int cuiInternalLedPin = 13;

// Calculation constants.
const float cfCrankLength = 0.1f;   // in m.
const float cfPi = 3.1416f;
const unsigned int cuiMinRevDuration = 240;   // in ms.
const unsigned int cuiMaxRevDuration = 2000;  // in ms.
const unsigned int cuiMaxForce = 2000;  // In Newtons.

// Timer constants
const unsigned long culTimerInterruptPeriod = 1000;     // in us.
const unsigned long culTimerCadenceReadPeriod = 10;     // in ms.
const unsigned long culTimerTorqueReadPeriod = 100;     // in ms.

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
unsigned long ulCadenceMillisecondCounter = 0;
unsigned long ulTorqueMillisecondCounter = 0;
unsigned int uiCadenceReadWait = cuiMinRevDuration;

// Physical quantities.
float fInstTorque = 0.0f;
float fAverageTorque = 0.0f;
float fInstCadenceRev = 0.0f;
float fAveragePower = 0.0f;

// Cadence related variables
unsigned int uiCumulativeCrankRevolutions = 0;
unsigned int uiOldCumulativeCrankRevolutions = 0;
unsigned long ulCadenceLastCrankEventTime = 0;
unsigned long ulOldCadenceLastCrankEventTime = 0;
unsigned long usReferenceMillis;
unsigned int uiCadenceRevDuration = 0;
unsigned short usOldCadenceFeature = 0;

// Torque related variables
float fTorqueSum = 0.0f;
unsigned short usTorqueCounter = 0;
float fAccumulatedTorque = 0;
float fOldAccumulatedTorque = 0;
unsigned int oldBlePowerFeature = 0;
unsigned char oldBleSensorLocation = 0;

// BLE Formatted global variables
unsigned short bleCumulativeCrankRevolutions = 0; // Resolution: 1 bit = 1 revolution.
unsigned short bleLastCrankEventTime = 0;         // Resolution: 1 bit = 1/1024 seconds.
short blePowerMeasurement = 0;           // Resolution: 1 bit = 1 Watt.
unsigned short bleAccumulatedTorque = 0;          // Resolution: 1 bit = 1/32 Nm.

// BLE Peripheral, service and caracteristic.
BLEPeripheral blePeripheral; // create peripheral instance.

// BLE Cycing power initialization.
BLEService powerService("1818");
BLECharacteristic powerMeasurementChar("2A63", BLENotify, 10);
//BLECharacteristic powerMeasurementClientDesc("2902", BLERead | BLEWrite, 2);
//BLECharacteristic powerMeasurementServerDesc("2903", BLERead | BLEWrite, 2);
BLECharacteristic powerFeatureChar("2A65", BLERead, 4);
BLECharacteristic powerSensorLocationChar("2A5D", BLERead, 1);

// BLE Cycling speed and cadence initialization.
BLEService cadenceService("1816");
BLECharacteristic cadenceMeasurementChar("2A5B", BLENotify, 5);
//BLECharacteristic cadenceMeasurementDesc("2902", BLERead | BLEWrite, 2);
BLECharacteristic cadenceFeatureChar("2A5C", BLERead, 2);

/// END DEFINITIONS ///

/**
    Setup function, run at the beginning of execution
*/
void setup() {
  // Start serial debugging.
  Serial.begin(115200);

  // Initialize digital pin behaviour.
  pinMode(cuiCadencePin, INPUT);
  pinMode(cuiInternalLedPin, OUTPUT);

  // Initialize BLE device, services and characteristics.
  initBleDevice();

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
    digitalWrite(cuiInternalLedPin, HIGH);  // turn on the LED to indicate the connection

    // Loop here while central is still connected
    while (central.connected()) {

      // Update power and cadence characteristics every second.
      Serial.println("Running");
      BLE_CAD_updateCadence();
      BLE_POW_updatePower();

#ifdef DEBUG_BLE_PERIPHERAL
      Serial.print("BLEPeripheral: ");
      Serial.println(blePeripheral.connected());
      Serial.print("BLECentral: ");
      Serial.println(central.connected());
#endif

      delay(1000);
    }

    // when the central disconnects, turn off the LED and reset the flag.
    bConnectedFlag = 0;
    digitalWrite(cuiInternalLedPin, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    central.disconnect();
    /*
        // Reset BLE device.
        blePeripheral.end();
        initBleDevice();
        // Advertise the service
        blePeripheral.begin();
    */
    Serial.println("Bluetooth device active, waiting for connections...");
  }
}

void Int_1ms(void) {
  if (bConnectedFlag) {
    ulCadenceMillisecondCounter++;
    ulTorqueMillisecondCounter++;

    if (ulCadenceMillisecondCounter >= culTimerCadenceReadPeriod) {
      ulCadenceMillisecondCounter = 0;
      SEN_CAD_getInstantCadence();
    }

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

  // Initialize cadence service.
  blePeripheral.addAttribute(cadenceService);
  blePeripheral.addAttribute(cadenceMeasurementChar);
  blePeripheral.addAttribute(cadenceFeatureChar);

  // Initialize timer interruptions.
  CurieTimerOne.start(culTimerInterruptPeriod, &Int_1ms);

  // Advertise the service
  blePeripheral.begin();
  Serial.println("Bluetooth device active, waiting for connections...");
  delay(1000);
}


