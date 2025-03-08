#include <Arduino.h>
#include "tasks.h"
bool SD_INT_STATUE;
SemaphoreHandle_t sdCardMutex; // Mutex to protect SD card access
String ssid = "404";
String password = "abcd0404";
#include <esp_heap_caps.h>  // To use PSRAM
void setup()
{
  // Initialize watchdog timer
  esp_task_wdt_init(10, true); // Set timeout to 10 seconds
  esp_task_wdt_add(NULL);      // Add the current task to the watchdog

  psramInit();
  Serial.begin(115200);
  vTaskDelay(500);
  Serial.println(F("///////////////////////////////////////////////////////"));

  /* Init SD spi */
  SPIClass *sd_spi = new SPIClass(FSPI);
  sd_spi->begin(15, 16, 6, 5);
  SD_INT_STATUE = SD.begin(5, *sd_spi, 40000000);
  if (!SD_INT_STATUE)
  {
    Serial.println("SD card init failed");
    return;
  }
  Serial.println("SD card init success");
  /* Get SD card info */
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_MMC)
    Serial.print("MMC");
  else if (cardType == CARD_SD)
    Serial.print("SDSC");
  else if (cardType == CARD_SDHC)
    Serial.print("SDHC");
  else
    Serial.print("-ERROR-");
  Serial.printf(" %dGB\r\n", (SD.cardSize() / 1073741824));
  // Initialize SD card mutex
  sdCardMutex = xSemaphoreCreateMutex();
  if (sdCardMutex == NULL)
  {
    Serial.println("Failed to create SD card mutex.");
    while (1)
      ; // Halt if mutex creation fails
  }

  /*int wifi*/
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid.c_str(), password.c_str());

  xTaskCreatePinnedToCore(RGBtask, "RGB LED", 2 * 1024, NULL, 1, &RGB_TASK_HANDLE, 1);
  xTaskCreatePinnedToCore(memory_task, "Memory task", 2 * 1024, NULL, 1, NULL, 1);
  // xTaskCreatePinnedToCore(BLE_task,"BLUETOOTH task",4*1024,NULL,1,NULL,0);
  // xTaskCreatePinnedToCore(EvilApple,"EvilAppleJuice-ESP32-INO task",4*1024,NULL,1,NULL,0);
  // xTaskCreatePinnedToCore(USB_task,"USB task",4*1024,NULL,1,NULL,0);
  // vTaskDelay(1000);
  // xTaskCreatePinnedToCore(tft_task,"Display task",16*1024,NULL,1,NULL,1);
  //  tft_task(NULL);
  // xTaskCreatePinnedToCore(mic_task,"Display task",mic_taskMemory,NULL,1,&mic_task_HANDLE,1);
  // 连接HID蓝牙设备任务
  xTaskCreate(BLEDevice_connect, "BLEDevice_connect", BLEDevice_connect_taskMemory, NULL, 1, &BLEDevice_connect_HANDLE);
  // xTaskCreatePinnedToCore(LGFX_task, "Display task", LGFX_taskMemory, NULL, 1, &LGFX_task_HANDLE, 1);
  LGFX_task(NULL);
  //vTaskDelay(4000);
  //xTaskCreatePinnedToCore(audio_task, "audio_task", audio_taskMemory, NULL, 1, &audio_task_HANDLE, 1);
  // xTaskCreatePinnedToCore(printLyricsTask, "Print Lyrics Task", lrc_taskMemory, NULL, 1, &lrc_task_HANDLE, 1);
}

void loop()
{
  lv_timer_handler();
  //audio.loop();
  // Feed the watchdog to prevent reset
  esp_task_wdt_reset();
  vTaskDelay(1);
}
