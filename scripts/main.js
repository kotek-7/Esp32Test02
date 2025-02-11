/// <reference types="web-bluetooth" />

const SERVICE_UUID = "0a133f79-efe1-40c5-b4a5-cba5980d0d0f";

// クライアント側とサーバー側のプログラムでtx・rxが反転していることに注意！
const RX_CHARACTERISTIC_UUID = "7687561d-1dba-458f-9fb2-58e6b85208ef";
const TX_CHARACTERISTIC_UUID = "8c83ffae-8421-4581-9755-10c5efd53d10";

/** @type {BluetoothRemoteGATTCharacteristic} */
let rxCharacteristic;
/** @type {BluetoothRemoteGATTCharacteristic} */
let txCharacteristic;

window.addEventListener("load", init);

function init() {
  const searchButton = document.getElementById("search-button");
  searchButton.addEventListener("click", onSearchDeviceButtonClick);
}

async function onSearchDeviceButtonClick() {
  const device = await navigator.bluetooth.requestDevice({
    acceptAllDevices: true,
    optionalServices: [SERVICE_UUID],
  });
  console.log("[device selected]");
  console.log("device: ");
  console.log(device);

  const server = await device.gatt.connect();
  console.log("[connected to server]");
  console.log("server: ");
  console.log(server);

  const service = await server.getPrimaryService(SERVICE_UUID);
  console.log("service: ");
  console.log(service);

  rxCharacteristic = await service.getCharacteristic(RX_CHARACTERISTIC_UUID);
  console.log("rx characteristic: ");
  console.log(rxCharacteristic);

  txCharacteristic = await service.getCharacteristic(TX_CHARACTERISTIC_UUID);
  console.log("tx characteristic: ");
  console.log(txCharacteristic);

  rxCharacteristic.addEventListener('characteristicvaluechanged', onValueChange);
  rxCharacteristic.startNotifications();
}

/**
 * @param {Event} evt 
 */
async function onValueChange(evt) {
  /** @type {DataView} */
  const rxBuf = evt.target.value;
  const value = rxBuf.getUint32(rxBuf, true);
  console.log("value: " + value);

  const rxValues = document.getElementById("rx-values");
  const rxValue = document.createElement("p");
  rxValue.innerHTML = value;
  rxValues.appendChild(rxValue);
}