#include <Arduino.h>
#include "tasks.h"

String ssid = "404";
String password = "abcd0404";
#include <esp_heap_caps.h> // To use PSRAM
void setup()
{
  // Initialize watchdog timer
  esp_task_wdt_init(10, true); // Set timeout to 10 seconds
  esp_task_wdt_add(NULL);      // Add the current task to the watchdog

  psramInit();
  Serial.begin(115200);
  vTaskDelay(500);
  Serial.println(F("///////////////////////////////////////////////////////"));

  xTaskCreatePinnedToCore(rtc_task, "RTC task", 2 * 1024, NULL, 1, NULL, 1);

  sd_setup();

  /*int wifi*/
  // WiFi.mode(WIFI_STA);
  // WiFi.disconnect();
  // WiFi.begin(ssid.c_str(), password.c_str());

  // xTaskCreatePinnedToCore(RGBtask, "RGB LED", 2 * 1024, NULL, 1, &RGB_TASK_HANDLE, 1);
  xTaskCreatePinnedToCore(memory_task, "Memory task", 2 * 1024, NULL, 1, NULL, 1);
  // xTaskCreatePinnedToCore(BLE_task,"BLUETOOTH task",4*1024,NULL,1,NULL,0);
  // xTaskCreatePinnedToCore(EvilApple,"EvilAppleJuice-ESP32-INO task",4*1024,NULL,1,NULL,0);
  // xTaskCreatePinnedToCore(USB_task,"USB task",4*1024,NULL,1,NULL,0);
  // vTaskDelay(1000);
  // xTaskCreatePinnedToCore(tft_task,"Display task",16*1024,NULL,1,NULL,1);
  // tft_task(NULL);
  // xTaskCreatePinnedToCore(mic_task,"Display task",mic_taskMemory,NULL,1,&mic_task_HANDLE,1);
  // 连接HID蓝牙设备任务
  xTaskCreatePinnedToCore(BLEDevice_connect, "BLEDevice_connect", BLEDevice_connect_taskMemory, NULL, 1, &BLEDevice_connect_HANDLE,0);
  // xTaskCreatePinnedToCore(LGFX_task, "Display task", LGFX_taskMemory, NULL, 1, &LGFX_task_HANDLE, 1);
  LGFX_task(NULL);
  // vTaskDelay(4000);
  // xTaskCreatePinnedToCore(audio_task, "audio_task", audio_taskMemory, NULL, 1, &audio_task_HANDLE, 1);
  // xTaskCreatePinnedToCore(printLyricsTask, "Print Lyrics Task", lrc_taskMemory, NULL, 1, &lrc_task_HANDLE, 1);
  xTaskCreatePinnedToCore(i2c_task, "i2c_task", 2 * 1024, NULL, 1, &i2c_task_HANDLE, 1);
  vTaskDelay(1000);
  xTaskCreatePinnedToCore(pca9554_task, "pca9554_task", 2 * 1024, NULL, 1, NULL, 1);
  //xTaskCreatePinnedToCore(QMC5883_task, "QMC5883_task", 2 * 1024, NULL, 1, NULL, 1);
  xTaskCreate(axp2101_task, "axp2101_task", 2 * 1024, NULL, 1, NULL);

}

void loop()
{
  lv_timer_handler();
  // audio.loop();
  //Feed the watchdog to prevent reset
  esp_task_wdt_reset();
  vTaskDelay(10);
}
