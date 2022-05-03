#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define LED_PIN LED_BUILTIN
#define SERVICE_UUID        "d7204ebc-4bc5-4a6a-a7e5-7a86f998b7a9"
#define CHARACTERISTIC_UUID "9be42785-94ae-4c1b-aa02-cd87c1c647c8"

// This sets _base_ MAC address. The actual BLE MAC Address will be D0:90:AB:CD:EF:E2.
// To understand why, see https://docs.espressif.com/projects/esp-idf/en/v3.1.7/api-reference/system/base_mac_address.html#number-of-universally-administered-mac-address
uint8_t kBaseMACAddress[6] = {0xd0, 0x90, 0xab, 0xcd, 0xef, 0xe0};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("[MyServerCallbacks::onConnect] Client connected");
    }

    void onDisconnect(BLEServer* pServer) {
      Serial.println("[MyServerCallbacks::onConnect] Client disconnected - resuming advertising");
      BLEDevice::startAdvertising();
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() != 2) {
        Serial.printf("[MyCallbacks::onWrite] Unexpected written length: %d\n",
                      value.length());
        return;
      } else if (value[0] == 0x7f && value[1] == 0xab) {
        Serial.println("[MyCallbacks::onWrite] Turning LED ON");
        digitalWrite(LED_PIN, HIGH);
      } else if (value[0] == 0x4b && value[1] == 0x9d) {
        Serial.println("[MyCallbacks::onWrite] Turning LED OFF");
        digitalWrite(LED_PIN, LOW);
      } else {
        Serial.println("[MyCallbacks::onWrite] Invalid written value");
        return;
      }
    }
};

void setup() {
  Serial.begin(115200);

  if (esp_base_mac_addr_set(kBaseMACAddress) != ESP_OK) {
    Serial.println("[setup] Error setting MAC address");
  }

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  BLEDevice::init("ESP32 LED");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  delay(1000);
}
