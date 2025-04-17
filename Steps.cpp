#include "SparkFunLSM6DSO.h"
#include "Wire.h"
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLE2902.h>

// I2C pins for ESP32
const byte I2C_SDA = 21;
const byte I2C_SCL = 22;

const char* DEVICE_SERVICE_UUID = "6e400001-b5a3-f393-e0a9-e50e24dcca9e";
const char* STEP_CHARACTERISTIC_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e";

LSM6DSO motionSensor;
BLECharacteristic *stepCharacteristic;

int steps = 0;
float threshold = 1.2; // Adjust this value after testing
bool aboveThreshold = false;

void performCalibration() {
  float readingSum = 0.0;
  Serial.println("Calibrating...");
  
  // Take 100 readings to find average resting position
  for (int i = 0; i < 100; i++) {
    float x = motionSensor.readFloatAccelX();
    float y = motionSensor.readFloatAccelY();
    readingSum += sqrt(x * x + y * y);  // Calculate movement magnitude
    delay(15);
  }

  steps = readingSum / 100;
  Serial.print("Baseline: ");
  Serial.println(steps);
}

void setupBLE() {
  // Initialize BLE with device name
  BLEDevice::init("StepTracker");
  BLEServer *server = BLEDevice::createServer();
  BLEService *service = server->createService(DEVICE_SERVICE_UUID);

  stepCharacteristic = service->createCharacteristic(
    STEP_CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  stepCharacteristic->addDescriptor(new BLE2902());
  stepCharacteristic->setValue("0");

  service->start();
  BLEAdvertising *advertiser = BLEDevice::getAdvertising();
  advertiser->addServiceUUID(DEVICE_SERVICE_UUID);
  BLEDevice::startAdvertising();
  Serial.println("BLE ready");
}

void sendSteps() {
  char buffer[16];
  snprintf(buffer, sizeof(buffer), "%d", steps);
  stepCharacteristic->setValue(buffer);
  stepCharacteristic->notify();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(50);

  // Check if motion sensor is connected
  if (!motionSensor.begin()) {
    Serial.println("IMU fail!");
    while (1);
  }

  motionSensor.initialize(BASIC_SETTINGS);  // Default sensor settings
  performCalibration();  // Get baseline readings
  setupBLE();  // Start Bluetooth
  sendSteps();  // Send initial zero value
}

void loop() {
  // Read current acceleration values
  float x = motionSensor.readFloatAccelX();
  float y = motionSensor.readFloatAccelY();
  float mag = sqrt(x * x + y * y);  // Calculate movement intensity

  // Step detection logic
  if (mag > threshold && !aboveThreshold) {
    aboveThreshold = true;
    steps++;
    Serial.print("Steps: ");
    Serial.println(steps);
    sendSteps();  // Update connected devices
  }

  // Reset detection when movement subsides
  if (mag < threshold) {
    aboveThreshold = false;
  }

  delay(20);
}
