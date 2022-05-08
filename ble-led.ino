#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#define LED_PIN LED_BUILTIN
#define SERVICE_UUID "d7204ebc-4bc5-4a6a-a7e5-7a86f998b7a9"
#define WRITE_CHARACTERISTIC_UUID "9be42785-94ae-4c1b-aa02-cd87c1c647c8"
#define WRITE_NO_RESP_CHARACTERISTIC_UUID "59da6d93-99c4-48c0-8a6e-d02dfe235787"

// This sets _base_ MAC address. The actual BLE MAC Address will be
// D0:90:AB:CD:EF:E2. To understand why, see
// https://docs.espressif.com/projects/esp-idf/en/v3.1.7/api-reference/system/base_mac_address.html#number-of-universally-administered-mac-address
uint8_t kBaseMACAddress[6] = {0xd0, 0x90, 0xab, 0xcd, 0xef, 0xa0};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("[MyServerCallbacks::onConnect] Client connected");
  }

  void onDisconnect(BLEServer *pServer) {
    Serial.println(
        "[MyServerCallbacks::onConnect] Client disconnected - resuming "
        "advertising");
    BLEDevice::startAdvertising();
  }
};

class WriteCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() != 2) {
      Serial.printf("[WriteCallbacks::onWrite] Unexpected written length: %d\n",
                    value.length());
      return;
    } else if (value[0] == 0xab && value[1] == 0xab) {
      Serial.println("[WriteCallbacks::onWrite] Turning LED ON");
      digitalWrite(LED_PIN, HIGH);
    } else if (value[0] == 0xab && value[1] == 0xac) {
      Serial.println("[WriteCallbacks::onWrite] Turning LED OFF");
      digitalWrite(LED_PIN, LOW);
    } else {
      Serial.println("[WriteCallbacks::onWrite] Invalid written value");
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

  // Write characteristic.
  // Using the nRF Connect app, this will enable a "Request" type of write.
  BLECharacteristic *pWriteCharacteristic = pService->createCharacteristic(
      WRITE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_WRITE);
  pWriteCharacteristic->setCallbacks(new WriteCallbacks());

  // Write with no response characteristic.
  // Using the nRF Connect app, this will enable a "Command" type of write.
  BLECharacteristic *pWriteNoRespCharacteristic =
      pService->createCharacteristic(WRITE_NO_RESP_CHARACTERISTIC_UUID,
                                     BLECharacteristic::PROPERTY_WRITE_NR);
  pWriteNoRespCharacteristic->setCallbacks(new WriteCallbacks());
  
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  delay(1000);
}
