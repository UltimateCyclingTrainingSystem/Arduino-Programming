#include <CurieBLE.h>

/// START DEFINITIONS ///
// Own sensors pins.
#define FORCE_PIN A0
#define CADENCE_PIN 2

// Board led (used to indicate BLE connection).
#define INT_LED_PIN 13

// BLE CONSTANTS
#define SENSOR_LOC 0x08

// Global variables
unsigned short globalCrank = 0;
long previousMillis = 0;

// BLE Peripheral, service and caracteristic.
BLEPeripheral blePeripheral; // create peripheral instance.

// BLE Cycling power service initialization.
BLEService powerService("1818"); // create service.
BLECharacteristic powerFeatureChar("2A65", BLERead, 4);
BLECharacteristic powerMeasurementChar("2A63", BLENotify, 8);
BLECharacteristic powerSensorLocChar("2A5D", BLERead, 1);

// BLE Cycling speed and cadence initialization.

// BLE Heart rate initialization.

// BLE Battery level monitor initialization.

/// END DEFINITIONS ///
/**
 *  Setup function, run at the beginning of execution
 */
void setup() {
  Serial.begin(9600);
  pinMode(CADENCE_PIN, INPUT);
  pinMode(INT_LED_PIN, OUTPUT);
  
  // set the local name peripheral advertises
  blePeripheral.setLocalName("PowerMeterProtocol");
  // set the UUID for the service this peripheral advertises:
  blePeripheral.setAdvertisedServiceUuid(powerService.uuid());

  // add service and characteristics
  blePeripheral.addAttribute(powerService);
  blePeripheral.addAttribute(powerFeatureChar);
  blePeripheral.addAttribute(powerMeasurementChar);
  blePeripheral.addAttribute(powerSensorLocChar);

  // Add starting values of the characteristics
  updatePowerSensorLocChar();
  updatePowerFeatureChar();

  // advertise the service
  blePeripheral.begin();

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
        updatePowerMeasChar();     
      }
    }
    // when the central disconnects, turn off the LED:
    digitalWrite(INT_LED_PIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());

    blePeripheral.end();
    blePeripheral.begin();

    Serial.println("Bluetooth device active, waiting for connections...");
  }
}

void updatePowerMeasChar(void) {
  unsigned char charValue[8];
  short instPower;
  unsigned short crankRev;
  unsigned short crankLastEventTime;

  instPower = 500 + random(0,10);
  crankRev = globalCrank + 1;
  globalCrank = crankRev;
  crankLastEventTime = 1024;
  
  // Flags
  charValue[0] = (char) 32;
  charValue[1] = (char) 0;
  charValue[2] = (char) (instPower & 0x00FF);
  charValue[3] = (char) ((instPower & 0xFF00) >> 8);
  charValue[4] = (char) (crankRev & 0x00FF);
  charValue[5] = (char) ((crankRev & 0xFF00) >> 8);
  charValue[6] = (char) (crankLastEventTime & 0x00FF);
  charValue[7] = (char) ((crankLastEventTime & 0xFF00) >> 8);

  powerMeasurementChar.setValue(charValue, 8);
  
}

void updatePowerFeatureChar(void) {
  unsigned char charValue[4];

    // Flags
  charValue[0] = (char) 8;
  charValue[1] = (char) 0;
  charValue[2] = (char) 16;
  charValue[3] = (char) 0;

  powerFeatureChar.setValue(charValue, 4);
}

void updatePowerSensorLocChar(void) {
  unsigned char charValue[1];
  charValue[0] = (char) 8;
  powerSensorLocChar.setValue(charValue, 1);
}

