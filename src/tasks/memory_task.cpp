#include "memory_task.h"
TaskHandle_t audio_task_HANDLE;
TaskHandle_t mic_task_HANDLE;
TaskHandle_t RGB_TASK_HANDLE;
TaskHandle_t lrc_task_HANDLE;
TaskHandle_t LGFX_task_HANDLE;
TaskHandle_t BLEDevice_connect_HANDLE;
TaskHandle_t time_task_handle;
int mic_taskMemory = 4 * 1024;
int memory_taskMemory = 3 * 1024;
int audio_taskMemory = 8 * 1024;
int lrc_taskMemory = 4 * 1024;
int LGFX_taskMemory = 2 * 1024;
int BLEDevice_connect_taskMemory = 4 * 1024;
int time_task_size = 8 * 1024; // 经测试，稳定运行至少需要8kb

int waterMark;
void memory_task(void *pvParam)
{
    Serial.println(F("[FreeRTOS]Memory task begin"));

    Serial.printf("Total Heap Size: %d KBytes\n", ESP.getHeapSize() / 1024);
    Serial.printf("Free Heap Size: %d KBytes\n", ESP.getFreeHeap() / 1024);

    vTaskDelay(4000);
    /**/

    Serial.printf("Deafult free size: %d Kbytes\n", heap_caps_get_free_size(MALLOC_CAP_DEFAULT) / 1024);
    Serial.printf("PSRAM size: %d Kbytes\n", ESP.getPsramSize() / 1024);
    Serial.printf("PSRAM free size: %d Kbytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024);
    Serial.printf("Flash size: %d Kbytes\n", ESP.getFlashChipSize() / 1024);
    Serial.printf("Free sketch space: %u Kbytes\n", ESP.getFreeSketchSpace() / 1024);
    Serial.printf("\n");

    while (1)
    {
        Serial.printf("heap space Used: %d Kbytes\n", (ESP.getHeapSize() - ESP.getFreeHeap()) / 1024);
        Serial.printf("PSRAM used size: %d Kbytes\n", (ESP.getPsramSize() - heap_caps_get_free_size(MALLOC_CAP_SPIRAM)) / 1024);
        waterMark = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf("[memory_task] Free Memory: %d Bytes\n", waterMark);
        Serial.printf("[memory_task] Used Memory: %d Bytes\n", memory_taskMemory - waterMark);
        Serial.printf("\n");

        if (audio_task_HANDLE != NULL)
        {
            waterMark = uxTaskGetStackHighWaterMark(audio_task_HANDLE);
            Serial.printf("[audio_task] Free Memory: %d Bytes\n", waterMark);
            Serial.printf("[audio_task] Used Memory: %d Bytes\n", audio_taskMemory - waterMark);
            Serial.printf("\n");
        }

        if (mic_task_HANDLE != NULL)
        {
            waterMark = uxTaskGetStackHighWaterMark(mic_task_HANDLE);
            Serial.printf("[mic_task] Free Memory: %d Bytes\n", waterMark);
            Serial.printf("[mic_task] Used Memory: %d Bytes\n", mic_taskMemory - waterMark);
            Serial.printf("\n");
        }

        if (lrc_task_HANDLE != NULL)
        {
            waterMark = uxTaskGetStackHighWaterMark(lrc_task_HANDLE);
            Serial.printf("[LRC_task] Free Memory: %d Bytes\n", waterMark);
            Serial.printf("[LRC_task] Used Memory: %d Bytes\n", lrc_taskMemory - waterMark);
            Serial.printf("\n");
        }

        if (LGFX_task_HANDLE != NULL)
        {
            waterMark = uxTaskGetStackHighWaterMark(LGFX_task_HANDLE);
            Serial.printf("[LGFX_task] Free Memory: %d Bytes\n", waterMark);
            Serial.printf("[LGFX_task] Used Memory: %d Bytes\n", LGFX_taskMemory - waterMark);
            Serial.printf("\n");
        }

        if (BLEDevice_connect_HANDLE != NULL)
        {
            waterMark = uxTaskGetStackHighWaterMark(BLEDevice_connect_HANDLE);
            Serial.printf("[BLE_mouse_task] Free Memory: %d Bytes\n", waterMark);
            Serial.printf("[BLE_mouse_task] Used Memory: %d Bytes\n", BLEDevice_connect_taskMemory - waterMark);
            Serial.printf("\n");
        }

        if (time_task_handle != NULL)
        {
            waterMark = uxTaskGetStackHighWaterMark(time_task_handle);
            Serial.printf("[time page task] Free Memory: %d Bytes\n", waterMark);
            Serial.printf("[time page task] Used Memory: %d Bytes\n", time_task_size - waterMark);
            Serial.printf("\n");
        }

        vTaskDelay(4000);
    }

    vTaskDelete(NULL);
}