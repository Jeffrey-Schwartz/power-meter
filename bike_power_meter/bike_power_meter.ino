/*
  Insert Comments & Description here
*/

#include <ArduinoBLE.h>       // Bluetooth
#include <Arduino_LSM6DS3.h>  // IMU: Accelerometer & Gyroscope

#define delayTime 200         // Reporting rate (200 milliseconds)
#define batteryDelay 10000    // Reporting rate (10 seconds)

long report_time; // Last time (in ms) data reported
float Ax, Ay, Az; // Last value read from accelerometer
float Wx, Wy, Wz; // Last value read from gyroscope
float Strain;     // Last value read from strain gauge

bool runStatus;   // Running: measuring/sending new data; true/false
int batteryLevel; // Lastest battery level

BLEService powerMeterService("1818");
BLEFloatCharacteristic AxChar("2A65", BLERead | BLENotify);
BLEFloatCharacteristic AyChar("2A65", BLERead | BLENotify);
BLEFloatCharacteristic AzChar("2A65", BLERead | BLENotify);
BLEFloatCharacteristic WxChar("2A65", BLERead | BLENotify);
BLEFloatCharacteristic WyChar("2A65", BLERead | BLENotify);
BLEFloatCharacteristic WzChar("2A65", BLERead | BLENotify);
BLEFloatCharacteristic StrainChar("2A58", BLERead | BLENotify);
BLELongCharacteristic TimeChar("2A2B", BLERead | BLENotify);
BLEBoolCharacteristic StatusChar("2A31", BLERead | BLEWrite | BLENotify);
BLEUnsignedIntCharacteristic batteryLevelChar("2A19", BLERead | BLENotify);

void setup()
{
  // Initialize global variables
  Ax = 0.0f;
  Ay = 0.0f;
  Az = 0.0f;
  Wx = 0.0f;
  Wy = 0.0f;
  Wz = 0.0f;
  runStatus = true;
  batteryLevel = 0;

  // Initialize Serial connection
  Serial.begin(9600);
  while (!Serial);

  // Initialize IMU
  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // Initialize BLE
  if (!BLE.begin())
  {
    Serial.println("starting BLE failed!");
    while (1);
  }
  /* Set a local name for the BLE device
     This name will appear in advertising packets
     and can be used by remote devices to identify this BLE device
     The name can be changed but may be truncated based on space left in advertisement packet
  */
  BLE.setLocalName("Bike Power Meter");
  BLE.setAdvertisedService(powerMeterService);    // add the service UUID
  powerMeterService.addCharacteristic(AxChar);    // add the BLE characteristics
  powerMeterService.addCharacteristic(AyChar);
  powerMeterService.addCharacteristic(AzChar);
  powerMeterService.addCharacteristic(WxChar);
  powerMeterService.addCharacteristic(WyChar);
  powerMeterService.addCharacteristic(WzChar);
  powerMeterService.addCharacteristic(TimeChar);
  powerMeterService.addCharacteristic(StrainChar);
  powerMeterService.addCharacteristic(batteryLevelChar);
  powerMeterService.addCharacteristic(StatusChar);
  BLE.addService(powerMeterService);              // Add the BLE service
  AxChar.writeValue(Ax);                          // set initial value for this characteristic
  AyChar.writeValue(Ay);
  AzChar.writeValue(Az);
  WxChar.writeValue(Wx);
  WyChar.writeValue(Wy);
  WzChar.writeValue(Wz);
  StrainChar.writeValue(Strain);
  batteryLevelChar.writeValue(batteryLevel);
  StatusChar.writeValue(runStatus);
  /* Start advertising BLE.  It will start continuously transmitting BLE
     advertising packets and will be visible to remote BLE central devices
     until it receives a new connection */
  // start advertising
  BLE.advertise();
  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  // wait for a BLE central
  BLEDevice central = BLE.central();

  // if a central is connected to the peripheral:
  if (central)
  {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    // turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);

    // print data header
    Serial.println("Time\tAx\tAy\tAz\tWx\tWy\tWz\tStrain");

    // Take new measurements and report every {delayTime/ms} while the central is connected:
    while (central.connected())
    {
      if (! runStatus)
        continue;
      long currentTime = millis();
      if ( (currentTime - report_time) >= delayTime)
      {
        report_time = currentTime;
        measure_data(currentTime);
      }
      if ( (currentTime - report_time) >= batteryDelay)
      {
        report_time = currentTime;
        updateBatteryLevel();
      }
    }
    // if the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

void measure_data(long currentTime)
{
  float x, y, z;
  String writeStr = "";

  // Read accelerometer data
  if (IMU.accelerationAvailable())
  {
    IMU.readAcceleration(x, y, z);
    if (x != Az)
    {
      Ax = x;
      AxChar.writeValue(Ax);  // and update the BLE characteristic value
    }
    if (y != Ay)
    {
      Ay = y;
      AyChar.writeValue(Ay);
    }
    if (z != Az)
    {
      Az = z;
      AzChar.writeValue(Az);
    }
  }
  writeStr = String(Ax,6) + "\t" + String(Ay,6) + "\t" + String(Az,6) + "\t";

  // Read gyroscope data
  if (IMU.gyroscopeAvailable())
  {
    IMU.readGyroscope(x, y, z);
    if (x != Wx)
    {
      Wx = x;
      WxChar.writeValue(Wx);
    }
    if (y != Wy)
    {
      Wy = y;
      WyChar.writeValue(Wy);
    }
    if (z != Wz)
    {
      Wz = z;
      WzChar.writeValue(Wz);
    }
  }
  writeStr += String(Wx,6) + "\t" + String(Wy,6) + "\t" + String(Wz,6) + "\t";

  // Read strain data
  if (true)
  {
    if (true)
    {
      Strain = 1.0f;
      StrainChar.writeValue(Strain);
    }
  }
  writeStr += String(Strain,6);

  writeStr = String(currentTime,6) + "\t" + writeStr;
  TimeChar.writeValue(currentTime);

  Serial.println(writeStr);
}

void updateBatteryLevel()
{
  /* Read the current voltage level on the A0 analog input pin.
     This is used here to simulate the charge level of a battery.
  */
  int battery = map(analogRead(A0), 0, 1023, 0, 100);
  if (batteryLevel != battery)
  {
    batteryLevel = battery;
    Serial.print("Battery Level % is now: ");
    Serial.println(batteryLevel);
    batteryLevelChar.writeValue(batteryLevel);
  }
}
