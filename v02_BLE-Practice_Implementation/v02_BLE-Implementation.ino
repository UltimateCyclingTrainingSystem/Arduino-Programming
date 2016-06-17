#include <CurieBLE.h>

BLEPeripheral blePeripheral;       // BLE Peripheral Device (the board you're programming)
BLEService cyclingSpeedAndCadence("1816"); // BLE Cycling Power Service
BLEService batteryService("180F");

// BLE Battery Level Characteristic
BLEUnsignedIntCharacteristic cscMeasurementChar("2A5B",
    BLENotify);
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",
    BLERead | BLENotify); 

int oldBatteryLevel = 0;
unsigned int oldCumCrankRevolutions = 0;  // last battery level reading from analog input
unsigned int oldMeasTime = 0;
long previousMillis = 0;

void setup() {
  Serial.begin(9600);    // initialize serial communication
  pinMode(13, OUTPUT);   // initialize the LED on pin 13 to indicate when a central is connected

  blePeripheral.setLocalName("cscMeasurements");
  blePeripheral.setAdvertisedServiceUuid(cyclingSpeedAndCadence.uuid());  // add the service UUID
  blePeripheral.addAttribute(cyclingSpeedAndCadence);   // Add the BLE Battery service
  blePeripheral.addAttribute(cscMeasurementChar); // add the cscMeasurement characteristic
  //blePeripheral.addAttribute(cscFeatureChar);

  blePeripheral.addAttribute(batteryService);   // Add the BLE Battery service
  blePeripheral.addAttribute(batteryLevelChar); // add the battery level characteristic
  
  cscMeasurementChar.setValue(oldCumCrankRevolutions);   // initial value for this characteristic
  batteryLevelChar.setValue(oldBatteryLevel);   // initial value for this characteristic

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
    digitalWrite(13, HIGH);

    // check the battery level every 200ms
    // as long as the central is still connected:
    while (central.connected()) {
      long currentMillis = millis();
      // if 200ms have passed, check the battery level:
      if (currentMillis - previousMillis >= 200) {
        previousMillis = currentMillis;
        increasePowerLevel();
        updateBatteryLevel();
      }
    }
    // when the central disconnects, turn off the LED:
    digitalWrite(13, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void increasePowerLevel() {
  unsigned int cumCrankRevolutions = oldCumCrankRevolutions + 1;

  if (cumCrankRevolutions != oldCumCrankRevolutions) {      // if the battery level has changed
    Serial.print("cumCrankRevolutions is now: "); // print it
    Serial.println(cumCrankRevolutions);
    Serial.println(sizeof(cumCrankRevolutions));
    cscMeasurementChar.setValue(cumCrankRevolutions);  // and update the battery level characteristic
    oldCumCrankRevolutions = cumCrankRevolutions;           // save the level for next comparison
  }
}


void updateBatteryLevel() {
  /* Read the current voltage level on the A0 analog input pin.
     This is used here to simulate the charge level of a battery.
  */
  int battery = analogRead(A0);
  int batteryLevel = map(battery, 0, 1023, 0, 100);

  if (batteryLevel != oldBatteryLevel) {      // if the battery level has changed
    Serial.print("Battery Level % is now: "); // print it
    Serial.println(batteryLevel);
    batteryLevelChar.setValue(batteryLevel);  // and update the battery level characteristic
    oldBatteryLevel = batteryLevel;           // save the level for next comparison
  }
}
