#include "tasks/BLE_mouse_task/BLE_TASK.h"

static BLEUUID serviceUUID("00001812-0000-1000-8000-00805f9b34fb"); // HID Service UUID
static BLEUUID charUUID("00002a4d-0000-1000-8000-00805f9b34fb");    // Report Characteristic UUID
BLEAdvertisedDevice *myDevice = nullptr;
BLEClient *pClient0 = nullptr;
bool doConnect = false;
bool isConnected = false;
void startScan(void *parameter);
bool connectTaskFinished = false;
///////// LVGL //////////////

extern int8_t mouseX;
extern int8_t mouseY;
extern bool mouseLeftButton;
static bool mouseRightButton = false;
extern uint8_t notifyCallback_statue;
extern bool mouse_indev_statue;
extern bool SD_INT_STATUE;

TaskHandle_t connectTaskHandle = NULL;
TaskHandle_t scanTaskHandle = NULL;

class MyClientCallback0 : public BLEClientCallbacks
{
    void onConnect(BLEClient *pClient0)
    {
        //Serial.println("Connected to BLE server.");
        mouse_indev_statue = true;
        isConnected = true;
    }

    void onDisconnect(BLEClient *pClient0)
    {
        //Serial.println("Disconnected from BLE server.");
        isConnected = false;
        doConnect = true;
        mouse_indev_statue = false;
    }
};

void notifyCallback0(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
    // Parse mouse data here
    mouseX = pData[1]; // X movement
    mouseY = pData[3]; // Y movement
    mouseLeftButton = pData[0] & 0x01;
    mouseRightButton = pData[0] & 0x02;
    notifyCallback_statue++;

    // //Serial.print("Mouse X: ");
    // //Serial.print(mouseX);
    // //Serial.print(" Y: ");
    // //Serial.println(mouseY);
}

class MyAdvertisedDeviceCallbacks0 : public NimBLEAdvertisedDeviceCallbacks
{
    void onResult(NimBLEAdvertisedDevice *advertisedDevice)
    {
        //Serial.print("BLE Advertised Device found: ");
        //Serial.println(advertisedDevice->toString().c_str());

        if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(serviceUUID))
        {
            if (!myDevice)
            {
                // 如果还没有设备准备连接，则保存该设备
                myDevice = new NimBLEAdvertisedDevice(*advertisedDevice);
            }
        }
    }
};

void connectToServer(void *parameter)
{
    //Serial.println("[BLE CONNECT TASK] connectToServer task created");
    while (true)
    {
        if (doConnect)
        {
            if (pClient0 == nullptr)
            {
                pClient0 = BLEDevice::createClient();
                pClient0->setClientCallbacks(new MyClientCallback0());
            }

            if (!isConnected)
            {
                // 尝试连接发现的设备
                if (myDevice && pClient0->connect(myDevice))
                {
                    //Serial.println("Connected to server.");

                    // Obtain a reference to the HID service
                    BLERemoteService *pRemoteService = pClient0->getService(serviceUUID);
                    if (pRemoteService == nullptr)
                    {
                        //Serial.println("Failed to find our service UUID.");
                        pClient0->disconnect();
                    }
                    else
                    {
                        // Obtain a reference to the Report characteristic
                        BLERemoteCharacteristic *pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
                        if (pRemoteCharacteristic == nullptr)
                        {
                            //Serial.println("Failed to find our characteristic UUID.");
                            pClient0->disconnect();
                        }
                        else
                        {
                            if (pRemoteCharacteristic->canNotify())
                            {
                                pRemoteCharacteristic->subscribe(true, notifyCallback0, false);
                                //Serial.println("Connected to server from discovered device.");
                            }
                            isConnected = true;
                        }
                    }
                }
                else
                {
                    //Serial.println("Failed to connect to discovered device.");
                }
            }

            doConnect = false;
            // 连接成功后停止扫描任务
            // if (isConnected && scanTaskHandle != NULL) {
            //    vTaskDelete(scanTaskHandle);
            //    scanTaskHandle = NULL;
            //}
        }

        if (!isConnected)
        {
            //Serial.println("Reconnecting in 5 seconds...");
            vTaskDelay(5000); // 重试连接间隔
        }
        else
        {
            break; // 连接成功，退出循环，任务结束
        }
    }
    connectTaskHandle = NULL;
    vTaskDelete(NULL); // 任务结束，删除自己
}

void startScan(void *parameter)
{
    //Serial.println("[BLEDevice_connect TASK] BLE scan task created");
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks0());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    while (!isConnected)
    {
        //Serial.println("BLE Scanning...");
        pBLEScan->start(10, false); // in seconds
        vTaskDelay(10000);
    }
    scanTaskHandle = NULL;
    vTaskDelete(NULL); // 连接成功，删除任务
}

void BLEDevice_connect(void *parameter)
{
    ////////// SETUP //////////////
    //Serial.println("[BLE TASK] Start");
    if (!SD_INT_STATUE)
    {
        //Serial.println("BLE TASK Failed");
        vTaskDelete(NULL);
        return;
    }

    BLEDevice::init("");
    xTaskCreate(startScan, "startScan", 4096, NULL, 1, &scanTaskHandle);
    xTaskCreate(connectToServer, "connectToServer", 8192, NULL, 1, &connectTaskHandle);
    ////////// LOOP //////////////
    for (;;)
    {
        if (!isConnected)
        {
            if (!doConnect)
            {
                doConnect = true;
            }

            if (scanTaskHandle == NULL)
            {
                xTaskCreate(startScan, "startScan", 4096, NULL, 1, &scanTaskHandle);
            }

            if (connectTaskHandle == NULL)
            {
                xTaskCreate(connectToServer, "connectToServer", 8 * 1024, NULL, 1, &connectTaskHandle);
            }
        }
        vTaskDelay(1000);
    }
}