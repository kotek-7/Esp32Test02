// 参考:
// https://qiita.com/takudooon/items/2ab77f22196504ff9597
// https://qiita.com/umi_kappa/items/dd3d7a27cf714971406e

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <BLE2901.h>

// https://www.uuidgenerator.net/
#define SERVICE_UUID "0a133f79-efe1-40c5-b4a5-cba5980d0d0f"
#define TX_CHARACTERISTIC_UUID "7687561d-1dba-458f-9fb2-58e6b85208ef"
#define RX_CHARACTERISTIC_UUID "8c83ffae-8421-4581-9755-10c5efd53d10"

bool deviceConnected = false;
bool mirroredDeviceConnected = false;
BLEServer *pServer;
BLECharacteristic *pTxCharacteristic;
BLECharacteristic *pRxCharacteristic;

class ServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    Serial.println("connected!");
    deviceConnected = true;
  }
  void onDisconnect(BLEServer *pServer)
  {
    Serial.println("disconnected!");
    deviceConnected = false;
  }
};

class TxCharacteristicCallbacks : public BLECharacteristicCallbacks
{
};

class RxCharacteristicCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    String value = pCharacteristic->getValue();
    int len = value.length();
    Serial.println("******");
    Serial.print("length: ");
    Serial.println(len);
    Serial.print("value: ");
    Serial.println(value);
    Serial.println("******");
  }
};

void setup()
{
  Serial.begin(115200);

  BLEDevice::init("esp32_for_BLE");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  // Serviceを作成
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // 送信用Characteristicを作成
  {
    pTxCharacteristic = pService->createCharacteristic(
        TX_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->setCallbacks(new TxCharacteristicCallbacks());

    // Client Characteristc Configuration Descriptor
    BLE2902 *pCccd = new BLE2902();
    pTxCharacteristic->addDescriptor(pCccd);

    // Client User Description Descriptor
    BLE2901 *pCudd = new BLE2901();
    pCudd->setDescription("a characteristic for clients to receive values");
    pTxCharacteristic->addDescriptor(pCudd);
  }

  // 受信用Characteristicを作成
  {
    pRxCharacteristic = pService->createCharacteristic(
        RX_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE);
    pRxCharacteristic->setCallbacks(new RxCharacteristicCallbacks());

    // Client User Description Descriptor
    BLE2901 *pCudd = new BLE2901();
    pCudd->setDescription("a characteristic for clients to transmit values");
    pRxCharacteristic->addDescriptor(pCudd);
  }

  // 通信開始
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop()
{
  // 接続中
  if (deviceConnected)
  {
    int random_num = random(255);
    Serial.print("random num: ");
    Serial.println(random_num);
    pTxCharacteristic->setValue(random_num);
    pTxCharacteristic->notify();
  }

  // 接続完了
  if (deviceConnected && !mirroredDeviceConnected)
  {
    mirroredDeviceConnected = deviceConnected;
  }

  // 切断完了
  if (!deviceConnected && mirroredDeviceConnected)
  {
    delay(500);
    pServer->startAdvertising();
    Serial.println("start advertising..");
    mirroredDeviceConnected = deviceConnected;
  }
  delay(2000);
}
