#if 0
#include "tasks/tft_task/tft_task.h"
#define LGFX_AUTODETECT


#if USE_LGFX == 1
extern LGFX_tft tft;
#endif

#if USE_eTFT == 1
extern TFT_eSPI tft;
#define TFT_BLK 39

TCPReceiver recv;

void tft_task(void *pvParam)
{
  Serial.println("[FreeRTOS] init lvgl...");
  lv_init();
  lv_port_disp_init();

  tft.fillScreen(TFT_BLACK);
  vTaskDelay(1000);

  tft.setTextColor(TFT_RED, TFT_BLACK, false);
  tft.setTextFont(2);
  tft.drawChar('R', 64, 32);
  tft.fillScreen(TFT_RED);
  vTaskDelay(1000);

  tft.drawChar('G', 64, 32);
  tft.fillScreen(TFT_GREEN);
  vTaskDelay(1000);

  tft.drawChar('B', 64, 32);
  tft.fillScreen(TFT_BLUE);
  vTaskDelay(1000);

  // lv_demo_benchmark();
  // lv_demo_stress();
  // 显示屏背光
  ledcSetup(8, 5000 /*freq*/, 10 /*resolution*/);
  ledcAttachPin(TFT_BLK, 10);
  analogReadResolution(10);
  ledcWrite(8, 512);

  int width = tft.width();
  int height = tft.height();
  tft.fillScreen(TFT_WHITE);

  Serial.println("WiFi begin.");
  tft.println("WiFi begin.");
  // 尝试连接到存储的AP
  // WiFi.mode(WIFI_MODE_STA);
  WiFi.begin("404", "abcd0404");

  // 等待10秒钟再连接
  for (int i = 0; WiFi.status() != WL_CONNECTED && i < 200; i++)
  {
    Serial.print(".");
    vTaskDelay(100);
  }

  Serial.println(String("IP:") + WiFi.localIP().toString());
  tft.println(WiFi.localIP().toString());

  setup_t s;
  tft.getSetup(s);

  int spi_freq = SPI_FREQUENCY;
  // 如果您进入80MHz的重启循环，请尝试降低到40MHz。
  // 如果（spi_freq> 40000000）spi_freq = 40000000;

  recv.setup(s.r0_x_offset + 2, s.r0_y_offset - 47, width, height, spi_freq, TFT_MOSI, TFT_MISO, TFT_SCLK, TFT_CS, TFT_DC);

  while (1)
  {
    recv.loop();
    // lv_timer_handler(); /* let the GUI do its work */
    // vTaskDelay(10);
  }
}

#endif

#endif