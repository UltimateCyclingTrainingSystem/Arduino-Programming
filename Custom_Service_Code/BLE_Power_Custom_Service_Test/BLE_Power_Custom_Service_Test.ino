#include <CurieBLE.h>

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

// Global variables.
float oldPower = 0.0f;
float oldTorque = 0.0f;
float oldCadence = 0.0f;
long previousMillis = 0;

// BLE Peripheral, service and caracteristic.
BLEPeripheral blePeripheral; // create peripheral instance.
BLEService powerService("19B20010-E8F2-537E-4F6C-D104768A1214"); // create service.
BLEFloatCharacteristic powerCharacteristic("19B20011-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify); // create power characteristic and allow remote device to read and be notified.

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

  powerCharacteristic.setValue(0.0f);

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

    // check the heart rate measurement every 200ms
    // as long as the central is still connected:
    while (central.connected()) {
      long currentMillis = millis();
      // if 200ms have passed, check the heart rate measurement:
      if (currentMillis - previousMillis >= 200) {
        previousMillis = currentMillis;
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
 *  Calculate new power value, update old value, and notify client of a change.
 */
void updatePowerMeasurement(void) {
  float power;
  power = oldPower + 1.1f;

  if(power >= 1000.0) {
    power = 0.0f;
  }
  
  if (power != oldPower) {      // if the power has changed
    Serial.print("Power is now: "); // print it
    Serial.println(power);
    powerCharacteristic.setValue(power);  // and update the heart rate measurement characteristic
    oldPower = power;           // save the level for next comparison
  }
}

