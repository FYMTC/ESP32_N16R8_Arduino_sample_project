#ifndef BLE_TASK_H_
#define BLE_TASK_H_
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <NimBLEDevice.h>
#include <NimBLEAdvertisedDevice.h>
#include "NimBLEEddystoneURL.h"
#include "NimBLEEddystoneTLM.h"
#include "NimBLEBeacon.h"
void BLEDevice_connect(void * parameter);

#endif