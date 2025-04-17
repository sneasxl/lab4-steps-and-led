#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

const int LED_PIN = 25;          // GPIO pin connected to LED
const int LED_RESISTOR = 220;    

// BLE Settings (use your existing UUIDs)
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define LED_CHAR_UUID       "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *ledCharacteristic;

// LED Control Callback
class LedCallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    
    if (value == "ON") {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED turned ON");
    } 
    else if (value == "OFF") {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED turned OFF");
    }
  }
};

void setup() {
  Serial.begin(115200);
  
  // LED Setup
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Start with LED off

  // BLE Setup
  BLEDevice::init("ESP32-LED-Control");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  ledCharacteristic = pService->createCharacteristic(
    LED_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  ledCharacteristic->setCallbacks(new LedCallback());

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();
  
  Serial.println("BLE Ready - Send 'ON' or 'OFF'");
}

void loop() {
  delay(1000); // Minimal delay
}
