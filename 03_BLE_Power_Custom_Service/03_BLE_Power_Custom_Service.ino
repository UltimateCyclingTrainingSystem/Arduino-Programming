#include <CurieBLE.h>

#define TORQUE_PIN 1
#define CADENCE_PIN 2
#define LED_PIN 13

float oldPower = 0.0f;
long previousMillis = 0;  // last time the heart rate was checked, in ms

BLEPeripheral blePeripheral; // create peripheral instance
BLEService powerService("19B10010-E8F2-537E-4F6C-D104768A1214"); // create service

// create switch characteristic and allow remote device to read and write
BLEFloatCharacteristic powerCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);

void setup() {
  Serial.begin(9600);
  pinMode(TORQUE_PIN, INPUT);
  pinMode(CADENCE_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // set the local name peripheral advertises
  blePeripheral.setLocalName("PowerMeterBLE");
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

void loop() {
  // listen for BLE peripherals to connect:
  BLECentral central = blePeripheral.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(LED_PIN, HIGH);

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
    digitalWrite(LED_PIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void updatePowerMeasurement(void) {
  float power;
  power = oldPower + 1.1f;

  if(power >= 1000.0) {
    power = 0.0f;
  }
  
  if (power != oldPower) {      // if the heart rate has changed
    Serial.print("Power is now: "); // print it
    Serial.println(power);
    powerCharacteristic.setValue(power);  // and update the heart rate measurement characteristic
    oldPower = power;           // save the level for next comparison
  }
}

