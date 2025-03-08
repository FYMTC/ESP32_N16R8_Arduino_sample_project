#ifndef _TASKS_H_
#define _TASKS_H_
/* SD card */
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include "RGB_tasks.h"
#include "memory_task.h"
// #include "tasks/BLE_task/BLE_task.h"
#include "tasks/EvilApple/EvilApple.h"
#include "tasks/tft_task/tft_task.h"
// #include "USB_task.h"
#include "tasks/audio_task/audio_task.h"
#include <Adafruit_XCA9554.h>
#include "mic_task.h"
#include "tasks/lrc_task/lrc_task.h"
#include "tasks/LGFX_task/LGFX_task.h"
#include "tasks/BLE_mouse_task/BLE_TASK.h"

#include <wifi.h>
#include <HTTPClient.h>

#include "esp_task_wdt.h"

extern TaskHandle_t RGB_TASK_HANDLE;
extern TaskHandle_t audio_task_HANDLE;
extern TaskHandle_t mic_task_HANDLE;
extern TaskHandle_t lrc_task_HANDLE;
extern TaskHandle_t LGFX_task_HANDLE;
extern TaskHandle_t BLEDevice_connect_HANDLE;

extern int mic_taskMemory;
extern int audio_taskMemory;
extern int lrc_taskMemory;
extern int LGFX_taskMemory;
extern int BLEDevice_connect_taskMemory;

extern Audio audio;

#endif /* _TASKS_H_ */